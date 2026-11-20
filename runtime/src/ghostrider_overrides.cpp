#include "game_overrides.h"
#include "ps2_runtime.h"

#include <iostream>

namespace
{
    void applyGhostRiderNtscUOverrides(PS2Runtime &runtime)
    {
        (void)runtime;
        std::cout << "[ghostcomp] Applied Ghost Rider (NTSC-U SLUS_213.06) game override hook." << std::endl;

        // Add Ghost Rider (SLUS_213.06) address-specific runtime overrides here during Phase 4 bring-up, e.g.:
        // ps2_game_overrides::bindAddressHandler(runtime, 0x00123456u, "ret0");
    }
}

// Entry point 0x0025B718 matches SLUS_213.06 (NTSC-U v1.01); crc32=0 matches any build with that name/entry.
PS2_REGISTER_GAME_OVERRIDE("GhostRider_NTSC_U", "SLUS_213.06", 0x0025B718u, 0u, applyGhostRiderNtscUOverrides)
