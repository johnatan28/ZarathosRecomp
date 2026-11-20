# Contributing to ZarathosRecomp

Thank you for your interest in contributing to **ZarathosRecomp**! Because this is a static recompilation and reverse-engineering project, all contributions **must strictly follow clean-room community guidelines**.

## Mandatory Legal Rules

1. **Never upload or commit copyrighted game files.**
   - Do **not** commit or attach PS2 ISOs, ELF binaries (`SLUS_*`, `SLES_*`), IRX modules, PS2 BIOS dumps, textures, models, audio, videos, or game scripts — not even in issues or pull requests.
2. **Never commit generated recompiled C++ code.**
   - All C++ files emitted by `ps2_recomp` belong exclusively to the local `generated/` directory (ignored by Git).
3. **Never commit proprietary SDK headers or leaked source code.**
   - Any runtime stubs, struct definitions, or function signatures must be derived cleanly from public documentation, open-source PS2 projects, or direct reverse-engineering evidence.

## What You Can Contribute

- Ghidra function boundaries, jump table fixes, and symbol maps (`.csv` / `.toml`) in `re/`.
- `PS2Recomp` configuration files and instruction patches in `re/`.
- Clean-room runtime stubs, HLE implementations, and game overrides in `runtime/`.
- Reverse-engineering documentation and verified findings in `context/03-re-notes.md`.

## Commit & PR Guidelines

- Keep pull requests focused on a single subsystem, symbol group, or bug fix.
- Always specify which **region/build** of the game your addresses apply to (e.g., NTSC-U `SLUS_213.06` vs. PAL `SLES_...`).
- Include verification evidence (address, Ghidra/REA observation, or runtime log) when renaming functions or adding stubs.
- Ensure `git status` shows zero files under `game/`, `generated/`, or Ghidra project databases (`*.gpr`, `*.rep/`) before committing.
