// Headless exporter for ghostcomp: exports PS2Recomp-compatible function CSV,
// ELF memory sections table, and engine/SDK string highlights from Ghidra.
// @category ghostcomp

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.data.StringDataInstance;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.mem.MemoryBlock;

import java.io.File;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.nio.file.Files;

public class ExportPS2Headless extends GhidraScript {

    private static class Record {
        String name;
        long start;
        long endExclusive;
        long size;
    }

    private static final Pattern ADDR_BIND_PATTERN = Pattern.compile("\"([A-Za-z0-9_.$]+)@0x([0-9A-Fa-f]+)\"");

    private Map<Long, String> loadAnalyzerNames(File tomlFile) {
        Map<Long, String> map = new HashMap<>();
        if (tomlFile == null || !tomlFile.isFile()) {
            return map;
        }
        try {
            List<String> lines = Files.readAllLines(tomlFile.toPath(), StandardCharsets.UTF_8);
            for (String line : lines) {
                Matcher m = ADDR_BIND_PATTERN.matcher(line);
                if (m.find()) {
                    String name = m.group(1);
                    long addr = Long.parseUnsignedLong(m.group(2), 16);
                    map.putIfAbsent(addr, name);
                }
            }
        } catch (Exception e) {
            println("Warning: could not parse analyzer TOML names: " + e.getMessage());
        }
        return map;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 3) {
            throw new IllegalArgumentException(
                "Usage: ExportPS2Headless.java <out_csv> <out_sections> <out_strings> [analyzer_toml]"
            );
        }

        File csvFile = new File(args[0]);
        File sectionsFile = new File(args[1]);
        File stringsFile = new File(args[2]);
        File analyzerToml = args.length >= 4 ? new File(args[3]) : null;

        Map<Long, String> analyzerNames = loadAnalyzerNames(analyzerToml);
        int renamedFromAnalyzer = 0;

        FunctionManager fm = currentProgram.getFunctionManager();
        FunctionIterator fit = fm.getFunctions(true);
        List<Record> records = new ArrayList<>();

        while (fit.hasNext() && !monitor.isCancelled()) {
            Function func = fit.next();
            AddressSetView body = func.getBody();
            if (body == null || body.getNumAddresses() == 0) {
                continue;
            }
            long start = func.getEntryPoint().getOffset();
            String currentName = func.getName();

            // Apply known SCE SDK names discovered by ps2_analyzer when Ghidra left a default name
            if (analyzerNames.containsKey(start) && currentName.startsWith("FUN_")) {
                String newName = analyzerNames.get(start);
                try {
                    func.setName(newName, ghidra.program.model.symbol.SourceType.USER_DEFINED);
                    currentName = func.getName();
                    renamedFromAnalyzer++;
                } catch (Exception ignored) {
                    // Keep existing name if duplicate/invalid
                }
            }

            Record rec = new Record();
            rec.name = currentName;
            rec.start = start;
            rec.endExclusive = body.getMaxAddress().getOffset() + 1L;
            rec.size = body.getNumAddresses();
            records.add(rec);
        }

        records.sort(Comparator.comparingLong(r -> r.start));

        csvFile.getParentFile().mkdirs();
        try (PrintWriter w = new PrintWriter(csvFile, StandardCharsets.UTF_8)) {
            w.println("Name,Start,End,Size");
            for (Record r : records) {
                w.printf("%s,0x%08X,0x%08X,%d%n", r.name, r.start, r.endExclusive, r.size);
            }
        }

        try (PrintWriter w = new PrintWriter(sectionsFile, StandardCharsets.UTF_8)) {
            w.println("# Memory blocks reported by Ghidra (" + currentProgram.getName() + ")");
            w.println("# Language: " + currentProgram.getLanguageID());
            w.println("# ImageBase: " + currentProgram.getImageBase());
            w.printf("%-24s %-12s %-12s %-10s %-5s %-5s%n", "Name", "Start", "End", "Size", "Perm", "Init");
            for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
                String perm = (block.isRead() ? "R" : "-")
                            + (block.isWrite() ? "W" : "-")
                            + (block.isExecute() ? "X" : "-");
                w.printf("%-24s 0x%08X   0x%08X   %-10d %-5s %-5s%n",
                    block.getName(),
                    block.getStart().getOffset(),
                    block.getEnd().getOffset(),
                    block.getSize(),
                    perm,
                    block.isInitialized() ? "yes" : "no"
                );
            }
        }

        int totalStrings = 0;
        List<String> interestingStrings = new ArrayList<>();
        DataIterator dit = currentProgram.getListing().getDefinedData(true);
        while (dit.hasNext() && !monitor.isCancelled()) {
            Data d = dit.next();
            if (!d.hasStringValue()) {
                continue;
            }
            StringDataInstance sdi = StringDataInstance.getStringDataInstance(d);
            String val = sdi.getStringValue();
            if (val == null) {
                continue;
            }
            totalStrings++;
            String cleaned = val.replace("\r", "\\r").replace("\n", "\\n").trim();
            if (cleaned.length() < 4 || cleaned.length() > 180) {
                continue;
            }
            String low = cleaned.toLowerCase(Locale.ROOT);
            boolean interesting = low.contains(".cpp") || low.contains(".c") || low.contains(".h")
                || low.contains("renderware") || low.contains("rws") || low.contains("bink")
                || low.contains("climax") || low.contains("ghost") || low.contains("gr.arc")
                || low.contains("cdrom0:") || low.contains("modules") || low.contains(".irx")
                || low.contains("sce") || low.contains("assert") || low.contains("build")
                || low.contains("version") || low.contains("mpeg") || low.contains("spu");
            if (interesting && interestingStrings.size() < 400) {
                Address addr = d.getAddress();
                interestingStrings.add(String.format("0x%08X: %s", addr.getOffset(), cleaned));
            }
        }

        try (PrintWriter w = new PrintWriter(stringsFile, StandardCharsets.UTF_8)) {
            w.println("# Ghidra string summary for " + currentProgram.getName());
            w.println("# Total defined strings: " + totalStrings);
            w.println("# Functions exported: " + records.size());
            w.println("# Functions renamed from ps2_analyzer SCE DB: " + renamedFromAnalyzer);
            w.println();
            for (String s : interestingStrings) {
                w.println(s);
            }
        }

        println("ExportPS2Headless complete: " + records.size() + " functions exported (" + renamedFromAnalyzer + " renamed from SCE DB), " + totalStrings + " strings scanned.");
    }
}
