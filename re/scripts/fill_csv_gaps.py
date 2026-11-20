#!/usr/bin/env python3
"""
Fills uncovered executable gaps in re/ghostrider_ghidra_functions.csv and
ensures every valid code pointer in .data / .rodata / .sdata (such as
RenderWare PS2All pipeline callbacks, C++ vtables, and .ctors) lands on an
exact function entry boundary for PS2Recomp static recompilation.
"""

from __future__ import annotations

import csv
import struct
import sys
from pathlib import Path

TEXT_START = 0x00100000
TEXT_END = 0x003A27E4
FILE_DELTA = 0x1000 - 0x00100000
JR_RA = 0x03E00008
BRANCH_OPS = {1, 4, 5, 6, 7, 17, 20, 21, 22, 23}
DATA_RANGES = (
    (0x003A5480, 0x00577F24),  # .data
    (0x00578000, 0x005AFC04),  # .rodata
    (0x005B0680, 0x005B10CC),  # .sdata
)


def read_word(data: bytes, va: int) -> int:
    return struct.unpack_from("<I", data, va + FILE_DELTA)[0]


def collect_data_code_pointers(data: bytes) -> set[int]:
    """Collect 4-byte-aligned pointers in initialized data sections pointing into .text."""
    ptrs: set[int] = set()
    for rng_start, rng_end in DATA_RANGES:
        for va in range(rng_start, rng_end + 1, 4):
            target = read_word(data, va)
            if (target & 3) != 0 or target < TEXT_START or target >= TEXT_END - 4:
                continue
            # Filter out obvious noise: require target word != 0 and either 8-byte aligned
            # (standard MIPS GCC function alignment) or preceded by NOP/jr $ra+delay.
            w0 = read_word(data, target)
            if w0 == 0:
                continue
            if (target & 7) == 0:
                ptrs.add(target)
            elif target >= TEXT_START + 8 and (
                read_word(data, target - 4) == 0 or read_word(data, target - 8) == JR_RA
            ):
                ptrs.add(target)
    return ptrs


def main() -> int:
    elf_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("game/SLUS_213.06")
    csv_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("re/ghostrider_ghidra_functions.csv")

    data = elf_path.read_bytes()
    data_ptrs = collect_data_code_pointers(data)

    rows: list[tuple[str, int, int, int]] = []
    with csv_path.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            start = int(row["Start"], 16)
            end = int(row["End"], 16)
            size = int(row["Size"])
            rows.append((row["Name"], start, end, size))
    rows.sort(key=lambda r: r[1])

    # Step 1: Drop accidental 8-byte jr $ra epilogue-only functions created inside a preceding uncovered gap
    cleaned: list[tuple[str, int, int, int]] = []
    removed_epilogues = 0
    for name, start, end, size in rows:
        prev_end = cleaned[-1][2] if cleaned else TEXT_START
        if (
            name.startswith("FUN_")
            and size == 8
            and start > prev_end
            and start not in data_ptrs
            and read_word(data, start) == JR_RA
            and read_word(data, start - 4) != 0
        ):
            removed_epilogues += 1
            continue
        cleaned.append((name, start, end, size))

    # Step 1b: If any existing FUN_ row swallowed a mandatory data pointer after a NOP or jr $ra+delay,
    # split it cleanly so the data pointer becomes an exact entry point (e.g., 0x002C7F40).
    split_rows: list[tuple[str, int, int, int]] = []
    split_count = 0
    for name, start, end, size in cleaned:
        if name.startswith("FUN_"):
            internal_splits = sorted(
                p for p in data_ptrs
                if start < p < end
                and (
                    read_word(data, p - 4) == 0
                    or (p >= start + 8 and read_word(data, p - 8) == JR_RA)
                )
            )
            if internal_splits:
                pts = [start, *internal_splits, end]
                for a, b in zip(pts, pts[1:]):
                    if b > a:
                        seg_name = name if a == start else f"FUN_{a:08x}"
                        split_rows.append((seg_name, a, b, b - a))
                split_count += len(internal_splits)
                continue
        split_rows.append((name, start, end, size))

    # Step 2: Walk every uncovered gap in .text and emit leaf/callback functions,
    # respecting mandatory data code pointer boundaries.
    final: list[tuple[str, int, int, int]] = []
    added_funcs = 0

    def emit_gap(gap_start: int, gap_end: int) -> None:
        nonlocal added_funcs
        va = gap_start
        while va + 8 <= gap_end:
            while va < gap_end and read_word(data, va) == 0:
                va += 4
            if va + 8 > gap_end:
                break

            fn_start = va
            fn_end = gap_end
            scan = fn_start
            max_branch_target = fn_start

            while scan + 4 < gap_end:
                # Split if the next word is a mandatory code pointer after NOP or jr $ra+delay
                if scan > fn_start and scan in data_ptrs and (
                    read_word(data, scan - 4) == 0
                    or (scan >= fn_start + 8 and read_word(data, scan - 8) == JR_RA)
                ):
                    fn_end = scan
                    break

                w = read_word(data, scan)
                op = (w >> 26) & 0x3F
                if op in BRANCH_OPS:
                    imm = w & 0xFFFF
                    if imm >= 0x8000:
                        imm -= 0x10000
                    bt = scan + 4 + imm * 4
                    if max_branch_target < bt < gap_end:
                        max_branch_target = bt
                if w == JR_RA and (scan + 8) > max_branch_target:
                    fn_end = scan + 8
                    break
                scan += 4

            if fn_end > fn_start:
                added_funcs += 1
                final.append((f"FUN_{fn_start:08x}", fn_start, fn_end, fn_end - fn_start))
            va = fn_end

    cursor = TEXT_START
    for row in split_rows:
        _, start, end, _ = row
        if start > cursor:
            emit_gap(cursor, start)
        final.append(row)
        if end > cursor:
            cursor = end
    if cursor < TEXT_END:
        emit_gap(cursor, TEXT_END)

    final.sort(key=lambda r: r[1])

    with csv_path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Name", "Start", "End", "Size"])
        for name, start, end, size in final:
            writer.writerow([name, f"0x{start:08X}", f"0x{end:08X}", size])

    print(
        f"fill_csv_gaps complete: data_ptrs={len(data_ptrs)}, removed_epilogues={removed_epilogues}, "
        f"split_boundaries={split_count}, added_gap_funcs={added_funcs} -> {len(final)} total functions."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
