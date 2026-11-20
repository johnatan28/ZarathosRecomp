<div align="center">

# ZarathosRecomp
### Ghost Rider (PS2) — Native PC Port & Static Recompilation

[![Platform](https://img.shields.io/badge/Platform-PlayStation%202%20%E2%86%92%20PC-orange?style=for-the-badge)](https://github.com/johnatan28/ZarathosRecomp)
[![Engine](https://img.shields.io/badge/Engine-RenderWare%203.7-red?style=for-the-badge)](https://github.com/johnatan28/ZarathosRecomp)
[![Recompiler](https://img.shields.io/badge/Recompiler-PS2Recomp-blue?style=for-the-badge)](https://github.com/ran-j/PS2Recomp)
[![License](https://img.shields.io/badge/License-GPL--3.0-green?style=for-the-badge)](./LICENSE)

</div>

**ZarathosRecomp** is an experimental static recompilation and native PC port project for **Ghost Rider (2007)** (PlayStation 2 — MIPS R5900 / Emotion Engine), built using **Ghidra**, **REA (Reverse Engineer Anything)**, and **[PS2Recomp](https://github.com/ran-j/PS2Recomp)**.

> **⚠️ Legal & Clean-Room Disclaimer**
>
> This repository **does NOT contain** any copyrighted game assets, ISO images, PS2 BIOS files, original ELF executables, or auto-generated recompiled C++ code derived from the game binary.
>
> To build and run `ZarathosRecomp`, you **must provide your own legally obtained copy** of *Ghost Rider* (PlayStation 2). This project is an independent, non-commercial preservation and technical engineering effort and is not affiliated with, endorsed by, or associated with Marvel, 2K Games, Climax Studios, or Sony Interactive Entertainment.

---

## Project Status

| Phase | Milestone | Status |
|-------|-----------|--------|
| **Phase 0** | Environment setup (Ghidra 12.2 + EmotionEngine + REA + PS2Recomp) | Completed |
| **Phase 1** | Target ELF metadata (`SLUS_213.06`), sections & initial Ghidra analysis | Completed |
| **Phase 2** | Symbol triage (`334` SCE SDK matches) & Ghidra + gap function map (`11,309` funcs) | Completed |
| **Phase 3** | Static recompilation (`11,211` recompiled, `98` stubs) & native `ZarathosRecomp.exe` build | Completed |
| **Phase 4** | Runtime / HLE bring-up (9 IOP `.IRX` drivers, RWA SIF HLE, live 640×448 double-buffered rendering!) | In Progress |
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
[ZarathosRecomp Runtime / Game Overrides + ps2xRuntime]
          │
          ▼
[Native PC Executable]
```

---

## Repository Layout

```text
ZarathosRecomp/
├── .github/                 # Issue templates & repository standards
├── AGENTS.md                # AI coding agent & contributor workflow guide
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
