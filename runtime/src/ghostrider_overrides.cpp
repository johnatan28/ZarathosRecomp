#include "game_overrides.h"
#include "ps2_runtime.h"
#include "ps2_stubs.h"
#include "ps2_syscalls.h"
#include "ps2_recompiled_functions.h"

#include <atomic>
#include <cstring>
#include <iostream>

namespace
{
    // 1. Heap isolation:
    // Ghost Rider's guest C++ allocator allocates upward from _end (0x00604F70).
    // Relocate ps2xRuntime's host-side guestMalloc pool to high RDRAM so stubs
    // (sceGsResetGraph, ps2xIOP) never overwrite Ghost Rider's STL/engine heap.
    constexpr uint32_t kHostSideGuestHeapBase = 0x01D00000u;
    constexpr uint32_t kHostSideGuestHeapLimit = 0x01EF0000u;

    std::atomic<uint32_t> g_rwaCoreStructAddr{0u};
    std::atomic<uint32_t> g_pendingRwaWakeupTid{0u};

    void writeGuestU32(uint8_t *rdram, uint32_t addr, uint32_t value)
    {
        if (!rdram || addr + 4u > 32u * 1024u * 1024u)
        {
            return;
        }
        std::memcpy(rdram + addr, &value, sizeof(value));
    }

    bool isValidGuestDataPtr(uint32_t addr)
    {
        return addr >= 0x00100000u && addr + 4u <= 0x02000000u && (addr & 3u) == 0u;
    }

    uint32_t getGprU32(const R5900Context *ctx, int reg)
    {
        return static_cast<uint32_t>(ctx->r[reg].m128i_u32[0]);
    }

    void setGprS32(R5900Context *ctx, int reg, int32_t value)
    {
        ctx->r[reg] = _mm_set_epi64x(0, static_cast<int64_t>(value));
    }

    void ensureIsolatedRuntimeHeap(PS2Runtime *runtime)
    {
        static std::atomic<bool> s_relocated{false};
        if (runtime && !s_relocated.exchange(true))
        {
            runtime->configureGuestHeap(kHostSideGuestHeapBase, kHostSideGuestHeapLimit);
            std::cout << "[ZarathosRecomp] Isolated ps2xRuntime host guestMalloc pool to [0x"
                      << std::hex << kHostSideGuestHeapBase << "..0x" << kHostSideGuestHeapLimit
                      << std::dec << "]." << std::endl;
        }
    }

    void hook_sceGsResetGraph(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        ensureIsolatedRuntimeHeap(runtime);
        ps2_stubs::sceGsResetGraph(rdram, ctx, runtime);
    }

    void hook_sceSifInitRpc(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        ensureIsolatedRuntimeHeap(runtime);
        ps2_stubs::sceSifInitRpc(rdram, ctx, runtime);
    }

    // 2. RenderWare Audio (RWA.IRX) SIF command HLE:
    // ps2xRuntime's sceSifSendCmd currently does not deliver cid=0x0 command packets to
    // RWA.IRX running inside ps2xIOP, causing RwaCoreInit (0x36B870) and RwaOsInitialize
    // (0x36D710) to spin waiting for IOP handshake replies. Capture the RWA core pointer
    // on entry to 0x36B870 and synthesize the handshake flags on RwaIopSendCommand (0x36BC20).
    void hook_rwaCoreInit_36b870(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        const uint32_t rwaCorePtr = getGprU32(ctx, 5); // $a1 = RWA core state ($s1 in 0x36D710)
        if (rwaCorePtr != 0u)
        {
            g_rwaCoreStructAddr.store(rwaCorePtr);
        }
        FUN_0036b870_0x36b870(rdram, ctx, runtime);
    }

    void hook_rwaSendCmd_36bc20(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        const uint32_t a0 = getGprU32(ctx, 4);
        const uint32_t a1 = getGprU32(ctx, 5);
        const uint32_t a2 = getGprU32(ctx, 6);
        const uint32_t cmd = getGprU32(ctx, 7); // $a3 = RWA SIF command ID
        const uint32_t t0 = getGprU32(ctx, 8);
        const uint32_t t1 = getGprU32(ctx, 9);  // $t1 = EE threadId to wake on synchronous cmds
        const uint32_t t2 = getGprU32(ctx, 10); // $t2 = optional resultOutPtr or param
        const uint32_t t3 = getGprU32(ctx, 11);
        const uint32_t gp = getGprU32(ctx, 28); // $gp = 0x005B7E70
        const uint32_t ra = getGprU32(ctx, 31);

        static std::atomic<uint32_t> s_logCount{0};
        if (++s_logCount <= 40u)
        {
            std::cout << "\n[ZarathosRecomp][RWA-HLE] cmd=0x" << std::hex << cmd
                      << " a0=0x" << a0 << " a1=0x" << a1 << " a2=0x" << a2
                      << " t0=0x" << t0 << " t1=0x" << t1 << " t2=0x" << t2 << " t3=0x" << t3
                      << " ra=0x" << ra << std::dec << std::endl;
        }

        // Always clear RWA DMA busy flag BEFORE entering 0x36BC20/0x36BCB0 so back-to-back
        // DMA uploads never queue behind a previous un-acknowledged transfer.
        writeGuestU32(rdram, gp - 0x6E58u, 0u);

        // Complete the EE-side SIF send normally first
        FUN_0036bc20_0x36bc20(rdram, ctx, runtime);

        // Synthesize the expected RWA.IRX reply flags so boot progresses past audio init
        switch (cmd)
        {
            case 0x01u: // Version query -> expects 0x116 (RWA 1.16) at gp - 0x6E4C
                writeGuestU32(rdram, gp - 0x6E4Cu, 0x00000116u);
                break;
            case 0x02u: // Init handshake -> expects non-zero at gp - 0x6E50
                writeGuestU32(rdram, gp - 0x6E50u, 1u);
                break;
            case 0x03u:
            case 0x04u:
            case 0x05u:
            case 0x06u:
            case 0x07u: // SPU2 wave / bank DMA upload -> clear busy flag (gp - 0x6E58)
            case 0x08u:
            case 0x09u:
            case 0x0Au:
            case 0x0Bu:
            case 0x0Cu:
                writeGuestU32(rdram, gp - 0x6E58u, 0u);
                break;
            case 0x11u: // Core open -> expects non-zero at gp - 0x6E20
                writeGuestU32(rdram, gp - 0x6E20u, 1u);
                break;
            case 0x13u: // SPU2 / IOP heap query -> expects gp - 0x6E28 != -1
                writeGuestU32(rdram, gp - 0x6E28u, 0x00080000u);
                writeGuestU32(rdram, gp - 0x6E24u, 0x00000000u);
                break;
            case 0x15u: // Audio core ready -> expects non-zero at rwaCore + 0x13A4
            {
                const uint32_t core = g_rwaCoreStructAddr.load();
                if (core != 0u)
                {
                    writeGuestU32(rdram, core + 0x13A4u, 1u);
                }
                break;
            }
            case 0x2Au: // Query/enable audio status -> writes non-zero to *t2 (gp - 0x6E10)
                if (isValidGuestDataPtr(t2))
                {
                    writeGuestU32(rdram, t2, 1u);
                }
                break;
            case 0x30u: // Sync config command (no return pointer checked)
                break;
            default:
                // For other synchronous RWA commands that pass a result output pointer in $t2
                // or $t3, we'll inspect the logged commands and specialize as needed.
                break;
        }

        // Synchronous RWA wrappers pass GetThreadId() in $t1 and immediately call SleepThread().
        // Arm a one-shot wakeup for that thread so SleepThread() returns immediately with KE_OK.
        if (t1 != 0u && t1 < 0x1000u)
        {
            g_pendingRwaWakeupTid.store(t1);
        }
    }

    // 0x0036BFB0: RwaDmaIsBusy() -> reads gp - 0x6E58.
    // Because 0x36BC20 tail-jumps to 0x36BCB0 (which sets gp - 0x6E58 = 1 after our
    // 0x36BC20 hook returns), report DMA completion here and clear gp - 0x6E58.
    void hook_rwaDmaIsBusy_36bfb0(uint8_t *rdram, R5900Context *ctx, PS2Runtime *)
    {
        const uint32_t gp = getGprU32(ctx, 28);
        writeGuestU32(rdram, gp - 0x6E58u, 0u);
        setGprS32(ctx, 2, 0);
        ctx->pc = getGprU32(ctx, 31);
    }

    void hook_SleepThread_399f00(uint8_t *rdram, R5900Context *ctx, PS2Runtime *runtime)
    {
        uint32_t expectedTid = g_pendingRwaWakeupTid.load();
        if (expectedTid != 0u)
        {
            R5900Context probe = *ctx;
            ps2_syscalls::GetThreadId(rdram, &probe, runtime);
            const uint32_t curTid = getGprU32(&probe, 2);
            if (curTid == expectedTid &&
                g_pendingRwaWakeupTid.compare_exchange_strong(expectedTid, 0u))
            {
                setGprS32(ctx, 2, 0); // KE_OK
                ctx->pc = getGprU32(ctx, 31);
                return;
            }
        }
        SleepThread_0x399f00(rdram, ctx, runtime);
    }

    void applyGhostRiderNtscUOverrides(PS2Runtime &runtime)
    {
        std::cout << "[ZarathosRecomp] Applied Ghost Rider (NTSC-U SLUS_213.06) game override hook." << std::endl;
        runtime.registerFunction(0x003175C8u, hook_sceGsResetGraph);
        runtime.registerFunction(0x0039CFB0u, hook_sceSifInitRpc);
        runtime.registerFunction(0x0036B870u, hook_rwaCoreInit_36b870);
        runtime.registerFunction(0x0036BC20u, hook_rwaSendCmd_36bc20);
        runtime.registerFunction(0x0036BFB0u, hook_rwaDmaIsBusy_36bfb0);
        runtime.registerFunction(0x00399F00u, hook_SleepThread_399f00);
    }
}

// Entry point 0x0025B718 matches SLUS_213.06 (NTSC-U v1.01); crc32=0 matches any build with that name/entry.
PS2_REGISTER_GAME_OVERRIDE("GhostRider_NTSC_U", "SLUS_213.06", 0x0025B718u, 0u, applyGhostRiderNtscUOverrides)
