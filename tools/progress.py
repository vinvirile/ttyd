#!/usr/bin/env python3
"""progress.py - show project progress in a human-readable format.

Reads build/G8MJ01/report.json and emits a multi-section progress report:
  - Overall project status (fuzzy %, functions, code, data, units)
  - Progress by category (DOL, RELs, SDK, MusyX, MSL_C, etc.)
  - Top matching units (highest fuzzy %)
  - Biggest untargeted pools (largest untouched units)
  - Recent improvements (highest unit gain since last run, optional)

Usage:
  python tools/progress.py                          # full report
  python tools/progress.py --top 20                 # show top 20 best/worst
  python tools/progress.py --category               # group by category
  python tools/progress.py --rel-only               # only REL modules
  python tools/progress.py --json                   # machine-readable

Compared to objdiff-cli's 'report' command, this tool:
  - Groups by category (DOL vs RELs vs SDK vs MusyX vs MSL)
  - Categorizes units by directory structure
  - Is a single Python file with no external dependencies
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"


def categorize(unit_name: str) -> str:
    """Return the category for a unit name."""
    if unit_name.startswith("rels/"):
        # Extract REL area name
        parts = unit_name.split("/")
        if len(parts) >= 2:
            return f"REL/{parts[1]}"
        return "REL/other"
    if unit_name.startswith("MarioSt/"):
        rest = unit_name[len("MarioSt/"):]
        parts = rest.split("/")
        if len(parts) >= 2:
            return f"DOL/{parts[0]}"
        return "DOL/root"
    if "MarioSt/MSL_C" in unit_name or "MSL_C" in unit_name:
        return "MSL_C"
    if "dolsdk" in unit_name.lower() or "DolphinLib" in unit_name or "SDK" in unit_name:
        return "SDK"
    if "musyx" in unit_name.lower() or "MusyX" in unit_name:
        return "MusyX"
    if unit_name.startswith("MarioSt/TRK"):
        return "TRK"
    return "other"


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--version", default=DEFAULT_VERSION)
    p.add_argument("--top", type=int, default=10, help="top N units to show (default 10)")
    p.add_argument("--category", action="store_true", help="group by category")
    p.add_argument("--rel-only", action="store_true", help="only show REL modules")
    p.add_argument("--json", action="store_true", help="emit JSON")
    p.add_argument("--untouched", action="store_true",
                   help="only show fully-untouched (zero percent fuzzy) units")
    args = p.parse_args()

    report_path = REPO / "build" / args.version / "report.json"
    if not report_path.exists():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())
    m = report["measures"]
    units = report["units"]

    if args.json:
        out = {
            "project": {
                "fuzzy_match_percent": m.get("fuzzy_match_percent", 0),
                "matched_code_percent": m.get("matched_code_percent", 0),
                "matched_functions_percent": m.get("matched_functions_percent", 0),
                "matched_code": m.get("matched_code", 0),
                "total_code": m.get("total_code", 0),
                "matched_data": m.get("matched_data", 0),
                "total_data": m.get("total_data", 0),
                "matched_functions": m.get("matched_functions", 0),
                "total_functions": m.get("total_functions", 0),
                "complete_units": m.get("complete_units", 0),
                "total_units": m.get("total_units", 0),
            },
            "units": units,
        }
        print(json.dumps(out, indent=2))
        return

    # Filter units
    if args.rel_only:
        units = [u for u in units if u["name"].startswith("rels/")]
    if args.untouched:
        units = [u for u in units
                 if (u.get("measures", {}).get("fuzzy_match_percent", 0) or 0) == 0
                 and u.get("measures", {}).get("total_functions", 0) > 0]

    # Overall summary
    print(f"=== TTYD-Decomp Progress ({args.version}) ===")
    print()
    print(f"  Fuzzy match:   {m.get('fuzzy_match_percent', 0):>6.2f}%")
    print(f"  Functions:     {m.get('matched_functions', 0):>6} / {m.get('total_functions', 0):<6} "
          f"({m.get('matched_functions_percent', 0):>5.1f}%)")
    print(f"  Code bytes:    {m.get('matched_code', 0):>6} / {m.get('total_code', 0):<6} "
          f"({m.get('matched_code_percent', 0):>5.1f}%)")
    print(f"  Data bytes:    {m.get('matched_data', 0):>6} / {m.get('total_data', 0):<6} "
          f"({m.get('matched_data_percent', 0):>5.1f}%)")
    print(f"  Complete units:{m.get('complete_units', 0):>6} / {m.get('total_units', 0):<6}")
    print()

    if args.category:
        # Group by category
        cats: dict[str, list[dict]] = defaultdict(list)
        for u in units:
            cats[categorize(u["name"])].append(u)

        print(f"  Progress by category:")
        print(f"  {'CATEGORY':<20} {'UNITS':>6} {'FN%':>6} {'CODE%':>6} {'FUZZY':>6}")
        cat_summaries = []
        for cat, cat_units in sorted(cats.items()):
            total_fns = sum(u.get("measures", {}).get("total_functions", 0) for u in cat_units)
            matched_fns = sum(u.get("measures", {}).get("matched_functions", 0) for u in cat_units)
            total_code = sum(u.get("measures", {}).get("total_code", 0) for u in cat_units)
            matched_code = sum(u.get("measures", {}).get("matched_code", 0) for u in cat_units)
            total_fuzzy = sum(
                (u.get("measures", {}).get("fuzzy_match_percent", 0) or 0)
                * u.get("measures", {}).get("total_code", 0)
                for u in cat_units
            )
            fn_pct = (matched_fns / total_fns * 100) if total_fns else 0
            code_pct = (matched_code / total_code * 100) if total_code else 0
            fuzzy_pct = (total_fuzzy / total_code) if total_code else 0
            cat_summaries.append((cat, len(cat_units), fn_pct, code_pct, fuzzy_pct))

        # Sort by code% (lowest first = most room for improvement)
        cat_summaries.sort(key=lambda x: x[3])
        for cat, n, fn_pct, code_pct, fuzzy_pct in cat_summaries:
            print(f"  {cat:<20} {n:>6} {fn_pct:>5.1f}% {code_pct:>5.1f}% {fuzzy_pct:>5.1f}%")
        print()

    # Top matching units (highest fuzzy)
    sorted_by_fuzzy = sorted(
        [u for u in units if (u.get("measures", {}).get("fuzzy_match_percent", 0) or 0) > 0],
        key=lambda u: u.get("measures", {}).get("fuzzy_match_percent", 0) or 0,
        reverse=True,
    )
    print(f"  Top {args.top} matching units (highest fuzzy %):")
    for u in sorted_by_fuzzy[:args.top]:
        ms = u.get("measures", {})
        print(f"    {ms.get('fuzzy_match_percent', 0):>5.1f}%  {u['name']}")
    print()

    # Largest untouched units (0% fuzzy, biggest total code)
    untouched = [
        u for u in units
        if (u.get("measures", {}).get("fuzzy_match_percent", 0) or 0) == 0
        and u.get("measures", {}).get("total_functions", 0) > 0
    ]
    untouched.sort(
        key=lambda u: u.get("measures", {}).get("total_code", 0),
        reverse=True,
    )
    print(f"  Top {args.top} largest untouched units:")
    for u in untouched[:args.top]:
        ms = u.get("measures", {})
        print(f"    {ms.get('total_functions', 0):>3} fns  "
              f"{ms.get('total_code', 0):>6} bytes  {u['name']}")
    print()


if __name__ == "__main__":
    main()
