// Recovers indirect call targets (.ctors, C++ vtables, RenderWare lui+addiu callbacks,
// and uncovered .text gap functions) in an already-analyzed Ghidra PS2 project,
// creates missing Function entries, and re-exports the PS2Recomp CSV map.
// @category ghostcomp

import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressIterator;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.program.model.symbol.ReferenceManager;

import java.io.File;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.TreeSet;

public class RecoverIndirectFunctions extends GhidraScript {

    private static class Record {
        String name;
        long start;
        long endExclusive;
        long size;
    }

    private boolean isStackPrologue(int word0) {
        int op = (word0 >>> 26) & 0x3F;
        int rs = (word0 >>> 21) & 0x1F;
        int rt = (word0 >>> 16) & 0x1F;
        int immSigned = (short) (word0 & 0xFFFF);
        return (op == 0x09 || op == 0x19) && rs == 29 && rt == 29 && immSigned < 0;
    }

    private boolean isLikelyFunctionPrologue(int word0, int word1) {
        if (word0 == 0x03E00008) {
            return true;
        }
        if (isStackPrologue(word0)) {
            return true;
        }
        if (word1 == 0x03E00008) {
            return true;
        }
        int op = (word0 >>> 26) & 0x3F;
        return op == 0x02;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            throw new IllegalArgumentException("Usage: RecoverIndirectFunctions.java <out_csv>");
        }
        File csvFile = new File(args[0]);

        Memory mem = currentProgram.getMemory();
        Listing listing = currentProgram.getListing();
        FunctionManager fm = currentProgram.getFunctionManager();
        ReferenceManager rm = currentProgram.getReferenceManager();

        MemoryBlock textBlock = mem.getBlock(".text");
        if (textBlock == null) {
            throw new IllegalStateException(".text memory block not found");
        }
        long textStart = textBlock.getStart().getOffset();
        long textEnd = textBlock.getEnd().getOffset();

        TreeSet<Long> candidateAddrs = new TreeSet<>();

        // 1. Scan .data / .rodata / .sdata for pointers into .text
        String[] scanBlocks = new String[] { ".data", ".rodata", ".sdata" };
        for (String blockName : scanBlocks) {
            MemoryBlock block = mem.getBlock(blockName);
            if (block == null || !block.isInitialized()) {
                continue;
            }
            long start = block.getStart().getOffset();
            long end = block.getEnd().getOffset() - 3;
            for (long va = start; va <= end && !monitor.isCancelled(); va += 4) {
                int raw = mem.getInt(toAddr(va));
                long target = raw & 0xFFFFFFFFL;
                if ((target & 3L) != 0L || target < textStart || target >= textEnd - 4) {
                    continue;
                }
                Address targetAddr = toAddr(target);
                if (fm.getFunctionContaining(targetAddr) != null) {
                    continue;
                }
                int w0 = mem.getInt(targetAddr);
                int w1 = mem.getInt(targetAddr.add(4));
                boolean inCtorsRange = (va >= 0x00577800L && va <= 0x00577C00L);
                if (inCtorsRange || isLikelyFunctionPrologue(w0, w1)) {
                    candidateAddrs.add(target);
                }
            }
        }

        int fromData = candidateAddrs.size();

        // 2. Collect Ghidra-resolved references (including lui+addiu RenderWare callbacks) landing in uncovered .text
        AddressSet textSet = new AddressSet(textBlock.getStart(), textBlock.getEnd());
        AddressIterator refDestIt = rm.getReferenceDestinationIterator(textSet, true);
        while (refDestIt.hasNext() && !monitor.isCancelled()) {
            Address targetAddr = refDestIt.next();
            long target = targetAddr.getOffset();
            if ((target & 3L) != 0L || target < textStart || target >= textEnd - 4) {
                continue;
            }
            if (fm.getFunctionContaining(targetAddr) != null) {
                continue;
            }
            int w0 = mem.getInt(targetAddr);
            if (w0 == 0) {
                continue;
            }
            // Accept any non-NOP target in uncovered .text referenced by code/data
            ReferenceIterator rit = rm.getReferencesTo(targetAddr);
            boolean hasRef = false;
            while (rit.hasNext()) {
                Reference ref = rit.next();
                if (!ref.isMemoryReference()) {
                    continue;
                }
                hasRef = true;
                break;
            }
            if (hasRef) {
                candidateAddrs.add(target);
            }
        }

        int fromRefs = candidateAddrs.size() - fromData;

        // 3. Scan ALL uncovered .text gaps for stack prologues and small leaf functions
        for (long va = textStart; va <= textEnd - 8 && !monitor.isCancelled(); va += 4) {
            Address addr = toAddr(va);
            if (fm.getFunctionContaining(addr) != null) {
                continue;
            }
            int w0 = mem.getInt(addr);
            if (w0 == 0) {
                continue;
            }
            int w1 = mem.getInt(addr.add(4));
            if (isStackPrologue(w0) || w0 == 0x03E00008) {
                candidateAddrs.add(va);
            } else if (w1 == 0x03E00008) {
                // Only treat 1-insn-before-jr-ra as function start if preceded by nop or inside another function's end
                if (va == textStart || mem.getInt(toAddr(va - 4)) == 0 || fm.getFunctionContaining(toAddr(va - 4)) != null) {
                    candidateAddrs.add(va);
                }
            }
        }

        int fromGaps = candidateAddrs.size() - fromData - fromRefs;
        println("RecoverIndirectFunctions: candidates -> data=" + fromData + ", refs=" + fromRefs + ", gaps=" + fromGaps + ", total=" + candidateAddrs.size());

        int disassembledCount = 0;
        int createdCount = 0;
        for (Long addrVal : candidateAddrs) {
            if (monitor.isCancelled()) {
                break;
            }
            Address addr = toAddr(addrVal);
            if (fm.getFunctionContaining(addr) != null) {
                continue;
            }
            if (listing.getInstructionAt(addr) == null) {
                DisassembleCommand disCmd = new DisassembleCommand(addr, textSet, true);
                if (disCmd.applyTo(currentProgram, monitor)) {
                    disassembledCount++;
                }
            }
            Instruction insn = listing.getInstructionAt(addr);
            if (insn == null) {
                continue;
            }
            if (fm.getFunctionContaining(addr) == null) {
                CreateFunctionCmd fnCmd = new CreateFunctionCmd(addr);
                if (fnCmd.applyTo(currentProgram, monitor) && fm.getFunctionAt(addr) != null) {
                    createdCount++;
                }
            }
        }

        println("RecoverIndirectFunctions: disassembled " + disassembledCount + " regions, created " + createdCount + " new functions.");

        // Export all functions in .text sorted by start address
        List<Record> records = new ArrayList<>();
        FunctionIterator fit = fm.getFunctions(true);
        while (fit.hasNext() && !monitor.isCancelled()) {
            Function func = fit.next();
            AddressSetView body = func.getBody();
            if (body == null || body.getNumAddresses() == 0) {
                continue;
            }
            long start = func.getEntryPoint().getOffset();
            if (start < textStart || start > textEnd) {
                continue;
            }
            Record rec = new Record();
            rec.name = func.getName();
            rec.start = start;
            rec.endExclusive = body.getMaxAddress().getOffset() + 1L;
            rec.size = body.getNumAddresses();
            records.add(rec);
        }

        records.sort(Comparator.comparingLong(r -> r.start));

        for (int i = 0; i + 1 < records.size(); i++) {
            Record cur = records.get(i);
            Record next = records.get(i + 1);
            if (cur.endExclusive > next.start && next.start > cur.start) {
                cur.endExclusive = next.start;
                cur.size = cur.endExclusive - cur.start;
            }
        }

        csvFile.getParentFile().mkdirs();
        try (PrintWriter w = new PrintWriter(csvFile, StandardCharsets.UTF_8)) {
            w.println("Name,Start,End,Size");
            for (Record r : records) {
                w.printf("%s,0x%08X,0x%08X,%d%n", r.name, r.start, r.endExclusive, r.size);
            }
        }

        println("RecoverIndirectFunctions complete: exported " + records.size() + " total functions to " + csvFile.getAbsolutePath());
    }
}
