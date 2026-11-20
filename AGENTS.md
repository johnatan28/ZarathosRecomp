# AGENTS.md — Contributor & AI Agent Guide for `ZarathosRecomp`

This file provides repository rules, architecture context, and workflow commands for AI coding agents and human contributors working on **ZarathosRecomp** — an experimental static recompilation of **Ghost Rider (2007)** (PlayStation 2, NTSC-U `SLUS_213.06`).

---

## 1. Mandatory Clean-Room & Legal Rules

1. **Never commit copyrighted game files** (ISOs, `SLUS_*`/`SLES_*` ELFs, `.IRX` modules, `GR.ARC`, `.BIK` videos, `.RWS` audio, PS2 BIOS dumps) or raw string dumps from `.rodata`.
2. **Never commit generated recompiled C++ files.** All outputs from `ps2_recomp` stay strictly inside local `generated/` (git-ignored).
3. **Never commit Ghidra databases or build artifacts** (`ghidra_project/`, `*.gpr`, `*.rep/`, `build/`, `tools/PS2Recomp/`).
4. **Evidence-first reverse engineering:** Before renaming symbols, editing `re/ghostrider.toml`, or adding runtime overrides in `runtime/src/ghostrider_overrides.cpp`, verify the exact guest address and behavior against `SLUS_213.06` (Ghidra / generated C++ / runtime logs).

---

## 2. Project Context & Architecture

Read the persistent project docs in `context/` before making architectural changes:

- `context/00-project-context.md` — target platform (`PS2` MIPS R5900), stack (`Ghidra 12.2` + `ghidra-emotionengine-reloaded` + `PS2Recomp`), clean-room principles.
- `context/01-roadmap.md` — phase-by-phase milestone tracker.
- `context/02-environment-setup.md` — local toolchain paths and prerequisites (Windows).
- `context/03-re-notes.md` — verified ELF metadata (`SLUS_213.06`, `entry = 0x0025B718`, `gp = 0x005B7E70`), section map, RenderWare 3.7 + Bink middleware inventory, and Phase 4 bring-up findings.

### Repository Layout

- `re/ghostrider.toml` — portable `PS2Recomp` configuration (uses relative paths).
- `re/ghostrider_ghidra_functions.csv` — Ghidra-backed + gap-recovered function map (`Name,Start,End,Size`).
- `re/scripts/ExportPS2Headless.java` — initial Ghidra headless R5900 analyzer/exporter.
- `re/scripts/RecoverIndirectFunctions.java` — recovers `.ctors`, vtables, and `lui+addiu` callbacks in Ghidra.
- `re/scripts/fill_csv_gaps.py` — ensures 100% `.text` coverage and splits swallowed `.data`/`.rodata`/`.sdata` code-pointer boundaries.
- `runtime/src/main.cpp` — native `ZarathosRecomp` runner entry point.
- `runtime/src/ghostrider_overrides.cpp` — `PS2_REGISTER_GAME_OVERRIDE` module for `SLUS_213.06` runtime hooks and bring-up diagnostics.
- `CMakeLists.txt` — builds `ZarathosRecomp.exe` using Unity Build + PCH + prebuilt `tools/PS2Recomp/b` Release libraries.

---

## 3. Standard Development Workflows

### Fast Runtime / Override Iteration (~10–15 seconds)
When editing only `runtime/src/ghostrider_overrides.cpp` or `runtime/src/main.cpp`:
```powershell
cmake --build build --config Release --parallel
```

### Updating Function Map & Recompiling Guest C++
When updating `re/ghostrider_ghidra_functions.csv` or `re/ghostrider.toml`:
```powershell
python re/scripts/fill_csv_gaps.py
.\tools\PS2Recomp\b\ps2xRecomp\Release\ps2_recomp.exe re\ghostrider.toml
cmake -S . -B build -DGHOSTCOMP_FAST_ITERATION=ON
cmake --build build --config Release --parallel
```

- Keep `-DGHOSTCOMP_FAST_ITERATION=ON` during Phase 4 bring-up (`/Od` on generated units while preserving `/MD` ABI compatibility with Release `ps2_runtime.lib`, cutting full rebuilds from ~35 min to ~5 min).
- Set `-DGHOSTCOMP_FAST_ITERATION=OFF` for full `/O2` optimized Release builds.

### Verifying Clean-Room Git Status Before Committing
Always verify `git status --short -uall` shows **zero** files under `game/`, `generated/`, `build/`, `ghidra_project/`, or `tools/PS2Recomp/` before committing or pushing.
