#include "game_overrides.h"
#include "ps2_runtime.h"
#include "ps2_stubs.h"

#include <atomic>
#include <iostream>

namespace
{
    // Ghost Rider's own guest C++ allocator allocates upward from _end (0x00604F70).
    // ps2xRuntime's host-side guestMalloc() (used by sceGsResetGraph / ps2xIOP) also starts
    // at SetupHeap's base (0x00604F70), which overwrote Ghost Rider's STL sentinels at
    // 0x00604F70/0x00604F80 with a 128-byte GIF packet (causing an infinite loop in FUN_00269c60).
    // Relocate ps2xRuntime's host-side guestMalloc pool to high RDRAM (29 MiB..30.9 MiB,
    // safely below the EE stack at ~0x01FBFF50) before any runtime stub allocates from it.
    constexpr uint32_t kHostSideGuestHeapBase = 0x01D00000u;
    constexpr uint32_t kHostSideGuestHeapLimit = 0x01EF0000u;

    void ensureIsolatedRuntimeHeap(PS2Runtime *runtime)
    {
        static std::atomic<bool> s_relocated{false};
        if (runtime && !s_relocated.exchange(true))
        {
            runtime->configureGuestHeap(kHostSideGuestHeapBase, kHostSideGuestHeapLimit);
            std::cout << "[ZarathosRecomp] Isolated ps2xRuntime host guestMalloc pool to [0x"
                      << std::hex << kHostSideGuestHeapBase << "..0x" << kHostSideGuestHeapLimit
                      << std::dec << "] (preserving guest heap at 0x00604F70+)." << std::endl;
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

    void applyGhostRiderNtscUOverrides(PS2Runtime &runtime)
    {
        std::cout << "[ZarathosRecomp] Applied Ghost Rider (NTSC-U SLUS_213.06) game override hook." << std::endl;
        runtime.registerFunction(0x003175C8u, hook_sceGsResetGraph);
        runtime.registerFunction(0x0039CFB0u, hook_sceSifInitRpc);
    }
}

// Entry point 0x0025B718 matches SLUS_213.06 (NTSC-U v1.01); crc32=0 matches any build with that name/entry.
PS2_REGISTER_GAME_OVERRIDE("GhostRider_NTSC_U", "SLUS_213.06", 0x0025B718u, 0u, applyGhostRiderNtscUOverrides)
