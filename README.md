# ghostcomp

**ghostcomp** is an experimental static recompilation project for **Ghost Rider (2007)** (PlayStation 2 — MIPS R5900 / Emotion Engine), targeting native PC execution using **Ghidra**, **REA (Reverse Engineer Anything)**, and **[PS2Recomp](https://github.com/ran-j/PS2Recomp)**.

> **⚠️ Legal & Clean-Room Disclaimer**
>
> This repository **does NOT contain** any copyrighted game assets, ISO images, PS2 BIOS files, original ELF executables, or auto-generated recompiled C++ code derived from the game binary.
>
> To build and run `ghostcomp`, you **must provide your own legally obtained copy** of *Ghost Rider* (PlayStation 2). This project is an independent, non-commercial preservation and technical engineering effort and is not affiliated with, endorsed by, or associated with Marvel, 2K Games, Climax Studios, or Sony Interactive Entertainment.

---

## Project Status

| Phase | Milestone | Status |
|-------|-----------|--------|
| **Phase 0** | Environment setup (Ghidra 12.2 + EmotionEngine + REA + PS2Recomp) | Completed |
| **Phase 1** | Target ELF metadata, sections & initial Ghidra analysis | Pending |
| **Phase 2** | Symbol triage, SDK identification & Ghidra function map export | Pending |
| **Phase 3** | Static recompilation (`ps2_recomp`) & clean native C++ build | Pending |
| **Phase 4** | Runtime / HLE bring-up (syscalls, memory, VFS, boot loop) | Pending |
| **Phase 5** | Graphics (GS/VU), audio (SPU2), input & gameplay verification | Pending |

Detailed progress and technical notes live in [`context/`](./context/).

---

## Architecture & Tooling

```text
[User-supplied legal PS2 copy]
          │
          ▼  (extract main ELF into game/)
[Ghidra 12.2 + ghidra-emotionengine-reloaded]  ◄──►  [REA (rea-agents MCP)]
          │
          ▼  (ExportPS2Functions.java -> re/*.toml, re/*.csv)
[PS2Recomp (ps2xAnalyzer + ps2xRecomp)]
          │
          ▼  (local C++ generation into generated/)
[ghostcomp Runtime / Game Overrides + ps2xRuntime]
          │
          ▼
[Native PC Executable]
```

---

## Repository Layout

```text
ghostcomp/
├── .github/                 # Issue templates & repository standards
├── context/                 # Project architecture, roadmap, setup guide & RE notes
├── game/                    # Local user-supplied ELF/assets (git-ignored)
├── re/                      # Ghidra scripts, symbol maps (.csv/.toml) & recomp configs
├── runtime/                 # Game-specific runtime hooks, overrides & HLE stubs
├── tools/                   # External tool checkouts (PS2Recomp is git-ignored / submodule-ready)
├── CONTRIBUTING.md
├── LICENSE                  # GNU GPL v3.0
└── README.md
```

---

## Getting Started

1. Follow [`context/02-environment-setup.md`](./context/02-environment-setup.md) to set up:
   - **Ghidra 12.2** + [`ghidra-emotionengine-reloaded`](https://github.com/chaoticgd/ghidra-emotionengine-reloaded)
   - **JDK 25 (64-bit)**
   - **REA** (`rea-agents`)
   - **PS2Recomp** (`ran-j/PS2Recomp`) built with CMake + MSVC / C++20
2. Place the main ELF extracted from your own legal PS2 copy of *Ghost Rider* inside `game/`.
3. Follow the milestones in [`context/01-roadmap.md`](./context/01-roadmap.md).

---

## Credits & Upstream Projects

- **[PS2Recomp](https://github.com/ran-j/PS2Recomp)** — PlayStation 2 static recompiler, analyzer, IOP emulator, and runtime (`GPL-3.0`).
- **[ghidra-emotionengine-reloaded](https://github.com/chaoticgd/ghidra-emotionengine-reloaded)** — Ghidra processor & loader extension for the PS2 Emotion Engine (MIPS R5900 / MMI / VU0).
- **[Ghidra](https://github.com/NationalSecurityAgency/ghidra)** — Open-source software reverse engineering suite.
- **[REA (Reverse Engineer Anything)](https://github.com/morluto/rea)** — Evidence-driven MCP bridge for AI-assisted reverse engineering.

---

## License

This project is licensed under the **GNU General Public License v3.0** — see [LICENSE](./LICENSE) for details.
