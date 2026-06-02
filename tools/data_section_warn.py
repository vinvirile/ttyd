#!/usr/bin/env python3
"""data_section_warn.py - identify files that likely cannot match due to .strtab.

TTYD's MWCC compiler places static symbols BEFORE global symbols in the
.strtab section, but the original binary has them in a different order
(usually declaration order). This is a fundamental compiler behavior that
cannot be controlled from source.

As a result, any file with a .data section (.data, .rodata, .sdata, .sdata2)
will have a different .strtab than the original, even with byte-identical
code and data. This means such files cannot pass the SHA1 hash check at
link time, even if every function is 100% matched.

This tool identifies files that have data sections, so you can:
  - Avoid wasting time trying to flip them to Matching
  - Focus on decompiling the data and getting the .c source complete
    (the code matching alone is a real win even without hash match)
  - Document the limitation

A file is considered "data-heavy" if its .o has any of:
  - .data section (initialized globals)
  - .rodata section (read-only data, including string literals)
  - .sdata section (small initialized globals)
  - .sdata2 section (small read-only data, including float literals)

Note: .text and .bss alone are FINE to match.

Usage:
  python tools/data_section_warn.py                  # full audit
  python tools/data_section_warn.py <unit>           # check one unit
  python tools/data_section_warn.py --matching-only  # only check currently-Matching files
  python tools/data_section_warn.py --json

The tool reads dtk's elf info output for each .o file to detect the
sections present.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"

DATA_SECTIONS = {".data", ".rodata", ".sdata", ".sdata2"}
CODE_SECTIONS = {".text"}
BSS_SECTIONS = {".bss", ".sbss"}


def get_obj_path(unit_name: str, version: str) -> Path:
    """Resolve a unit name to its original .o file path (same as function_order_check)."""
    if unit_name.startswith("rels/"):
        parts = unit_name.split("/")
        if len(parts) >= 3:
            return REPO / "build" / version / parts[1] / "obj" / f"{parts[2]}.o"
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        return REPO / "build" / version / "obj" / f"{rel}.o"
    return REPO / "build" / version / "obj" / f"{unit_name}.o"


def get_sections(dtk_path: Path, obj_path: Path) -> list[str]:
    """Use dtk elf info to get the list of section names in a .o file."""
    if not obj_path.exists():
        return []
    try:
        result = subprocess.run(
            [str(dtk_path), "elf", "info", str(obj_path)],
            capture_output=True, text=True, timeout=30,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return []
    if result.returncode != 0:
        return []

    sections = []
    in_sections = False
    for line in result.stdout.splitlines():
        stripped = line.strip()
        if stripped == "Sections:":
            in_sections = True
            continue
        if in_sections:
            if "|" not in stripped and stripped:
                in_sections = False
                continue
            if "|" in stripped:
                parts = [p.strip() for p in stripped.split("|")]
                if parts and parts[0] and not parts[0].startswith("Name"):
                    sections.append(parts[0])
    return sections


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("unit", nargs="?", help="check one unit")
    p.add_argument("--version", default=DEFAULT_VERSION)
    p.add_argument("--matching-only", action="store_true",
                   help="only check currently-Matching files")
    p.add_argument("--json", action="store_true", help="emit JSON")
    p.add_argument("--dtk", default=None, help="path to dtk executable")
    p.add_argument("--max", type=int, default=200, help="max results to print")
    args = p.parse_args()

    dtk_path = Path(args.dtk) if args.dtk else (REPO / "build" / "tools" / ("dtk.exe" if sys.platform == "win32" else "dtk"))
    if not dtk_path.exists():
        sys.exit(f"dtk not found at {dtk_path}")

    if args.unit:
        obj_path = get_obj_path(args.unit, args.version)
        sections = get_sections(dtk_path, obj_path)
        data_sections = [s for s in sections if s in DATA_SECTIONS]
        print(f"=== {args.unit} ===")
        print(f"  Object: {obj_path} {'(exists)' if obj_path.exists() else '(MISSING)'}")
        if sections:
            print(f"  Sections: {', '.join(sections)}")
        if data_sections:
            print(f"  DATA-HEAVY: has {', '.join(data_sections)}")
            print(f"  WARNING: This file likely cannot pass SHA1 hash check even with 100% code match,")
            print(f"           due to MWCC's static-before-global .strtab ordering.")
        else:
            print(f"  No data sections - safe to flip to Matching")
        return

    report_path = REPO / "build" / args.version / "report.json"
    if not report_path.exists():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())

    rows: list[dict] = []
    for u in report["units"]:
        name = u["name"]
        if u.get("metadata", {}).get("auto_generated"):
            continue
        if "unknown/autos" in name:
            continue
        if u.get("measures", {}).get("total_functions", 0) == 0:
            continue
        if args.matching_only and (u.get("measures", {}).get("fuzzy_match_percent", 0) or 0) < 99.5:
            continue
        obj_path = get_obj_path(name, args.version)
        if not obj_path.exists():
            continue
        sections = get_sections(dtk_path, obj_path)
        data_secs = [s for s in sections if s in DATA_SECTIONS]
        rows.append({
            "name": name,
            "fuzzy": u.get("measures", {}).get("fuzzy_match_percent", 0) or 0,
            "data_sections": data_secs,
            "all_sections": sections,
        })

    # Split by data-heavy status
    data_heavy = [r for r in rows if r["data_sections"]]
    clean = [r for r in rows if not r["data_sections"]]

    if args.json:
        print(json.dumps({"data_heavy": data_heavy, "clean": clean}, indent=2))
        return

    print(f"=== Data Section Audit ({args.version}) ===")
    print(f"  {len(data_heavy)} data-heavy files (cannot match due to .strtab ordering)")
    print(f"  {len(clean)} clean files (no data sections - safe to flip Matching)")
    print()
    print(f"  Top {args.max} data-heavy files (by fuzzy %):")
    data_heavy.sort(key=lambda r: r["fuzzy"], reverse=True)
    for r in data_heavy[:args.max]:
        print(f"    {r['fuzzy']:>5.1f}%  {r['name']:<60}  {', '.join(r['data_sections'])}")
    if not args.matching_only:
        print()
        print(f"  Top {args.max} clean files (no data sections):")
        clean.sort(key=lambda r: r["fuzzy"], reverse=True)
        for r in clean[:args.max]:
            print(f"    {r['fuzzy']:>5.1f}%  {r['name']}")


if __name__ == "__main__":
    main()
