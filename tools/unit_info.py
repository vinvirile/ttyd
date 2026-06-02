#!/usr/bin/env python3
"""unit_info.py - show detailed info for a single unit (compilation object).

For a given unit, shows:
  - Fuzzy match % (overall and per-function)
  - All functions in the unit, with their match status and size
  - Source file path
  - Asm file path
  - Number of matched vs total functions
  - Asm file size

Usage:
  python tools/unit_info.py <unit_name>
  python tools/unit_info.py "MarioSt/driver/swdrv"
  python tools/unit_info.py "rels/aji/aji_07"
  python tools/unit_info.py "MarioSt/manager/evtmgr" --show-asm
  python tools/unit_info.py "MarioSt/manager/evtmgr" --json
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"


def resolve_paths(unit_name: str, version: str) -> dict[str, Path | None]:
    """Resolve all relevant file paths for a unit."""
    if unit_name.startswith("rels/"):
        parts = unit_name.split("/")
        area, unit = parts[1], parts[2]
        return {
            "source": REPO / "rels" / area / "src" / f"{unit}.c",
            "asm": REPO / "build" / version / "asm" / f"{unit_name}.s",
            "obj_target": REPO / "build" / version / area / "obj" / f"{unit}.o",
            "obj_current": REPO / "build" / version / area / "src" / f"{unit}.o",
        }
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        # Note: build dir uses <version>/asm/<rel>.s, no MarioSt/ prefix
        return {
            "source": REPO / "src" / f"{rel}.c",
            "asm": REPO / "build" / version / "asm" / f"{rel}.s",
            "obj_target": REPO / "build" / version / "obj" / f"{rel}.o",
            "obj_current": REPO / "build" / version / "src" / f"{rel}.o",
        }
    return {
        "source": None, "asm": None, "obj_target": None, "obj_current": None,
    }


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("unit", help="unit name (e.g. MarioSt/driver/swdrv or rels/aji/aji_07)")
    p.add_argument("--version", default=DEFAULT_VERSION)
    p.add_argument("--json", action="store_true", help="emit JSON")
    p.add_argument("--show-asm", action="store_true", help="show first 30 lines of asm")
    p.add_argument("--max-fns", type=int, default=50, help="max functions to show (default 50)")
    args = p.parse_args()

    report_path = REPO / "build" / args.version / "report.json"
    if not report_path.exists():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())

    # Find the unit in the report
    unit_data = None
    for u in report["units"]:
        if u["name"] == args.unit:
            unit_data = u
            break

    if unit_data is None:
        sys.exit(f"unit '{args.unit}' not found in report.json")

    paths = resolve_paths(args.unit, args.version)
    measures = unit_data.get("measures", {})
    funcs = unit_data.get("functions", [])
    meta = unit_data.get("metadata", {})

    result = {
        "name": args.unit,
        "fuzzy_match_percent": measures.get("fuzzy_match_percent", 0),
        "matched_functions": measures.get("matched_functions", 0),
        "total_functions": measures.get("total_functions", 0),
        "matched_code_bytes": measures.get("matched_code", 0),
        "total_code_bytes": measures.get("total_code", 0),
        "matched_data_bytes": measures.get("matched_data", 0),
        "total_data_bytes": measures.get("total_data", 0),
        "files": {
            "source": str(paths["source"]) if paths["source"] and paths["source"].exists() else None,
            "asm": str(paths["asm"]) if paths["asm"] and paths["asm"].exists() else None,
            "obj_target_exists": paths["obj_target"].exists() if paths["obj_target"] else False,
            "obj_current_exists": paths["obj_current"].exists() if paths["obj_current"] else False,
        },
        "asm_size_bytes": paths["asm"].stat().st_size if paths["asm"] and paths["asm"].exists() else None,
        "functions": [],
        "metadata": meta,
    }
    for fn in funcs:
        vaddr = fn.get("metadata", {}).get("virtual_address", "")
        if vaddr and str(vaddr).isdigit():
            vaddr = f"0x{int(vaddr):08X}"
        result["functions"].append({
            "name": fn["name"],
            "size": fn.get("size", "0"),
            "fuzzy_match_percent": fn.get("fuzzy_match_percent", 0),
            "virtual_address": vaddr,
        })

    if args.json:
        print(json.dumps(result, indent=2))
        return

    print(f"=== {args.unit} ===")
    print(f"  Fuzzy match: {result['fuzzy_match_percent']:.1f}%")
    print(f"  Functions:   {result['matched_functions']}/{result['total_functions']} matched")
    print(f"  Code bytes:  {result['matched_code_bytes']}/{result['total_code_bytes']} matched")
    print(f"  Data bytes:  {result['matched_data_bytes']}/{result['total_data_bytes']} matched")
    print(f"  Source:      {result['files']['source'] or '(missing)'}")
    print(f"  ASM:         {result['files']['asm'] or '(missing)'}"
          + (f"  ({result['asm_size_bytes']} bytes)" if result['asm_size_bytes'] else ""))
    print(f"  Target .o:   {'EXISTS' if result['files']['obj_target_exists'] else 'MISSING'}")
    print(f"  Current .o:  {'EXISTS' if result['files']['obj_current_exists'] else 'MISSING'}")
    if meta:
        print(f"  Metadata:    {meta}")
    print()
    if funcs:
        print(f"  Functions (showing {min(len(funcs), args.max_fns)}/{len(funcs)}):")
        print(f"    {'FUNC':<40} {'SIZE':>6} {'FUZZY':>7}  {'VADDR'}")
        for fn in funcs[:args.max_fns]:
            print(f"    {fn['name']:<40} {fn.get('size', '0'):>6} "
                  f"{fn.get('fuzzy_match_percent', 0):>6.1f}%  "
                  f"{(fn.get('metadata', {}).get('virtual_address') or '')}")
    if args.show_asm and paths["asm"] and paths["asm"].exists():
        print()
        print(f"  --- First 30 lines of asm ---")
        for i, line in enumerate(paths["asm"].read_text(encoding="utf-8", errors="replace").splitlines()[:30]):
            print(f"  {i+1:>4}  {line}")


if __name__ == "__main__":
    main()
