#!/usr/bin/env python3
"""untouched_units.py - list 0% matched units in the project.

The single most reliable triage: list the units at 0% matched (fully
untouched, uncontended). At the last audit there were 700+ fully-untouched
units, a huge fresh backlog.

Distinguishes two flavors of 0%-matched unit:
  (A) MISSING-from-src (drift): the .c file doesn't exist or the function
      bodies are absent. These are the productive pool - add the asm symbol
      as a new correctly-named function; they routinely hit 90-100% from
      clean C.
  (B) present-but-unmatched: the function body is ALREADY in the .c
      (m2c/Ghidra-imported) but compiles to non-matching code. These are
      partial-improvement only; residuals are usually register-allocation/
      scheduling - i.e. asm-territory we skip. Low yield.

Usage:
  python tools/untouched_units.py                  # all 0% units
  python tools/untouched_units.py --max 30         # top 30 (smallest first)
  python tools/untouched_units.py --game-only      # exclude SDK/MarioSt/musyx
  python tools/untouched_units.py --flavor A       # only missing-from-src
  python tools/untouched_units.py --flavor B       # only present-but-unmatched
  python tools/untouched_units.py --csv            # emit CSV
  python tools/untouched_units.py --min-fn 2       # only units with 2+ functions

This is the SFA-style 'find genuinely-untouched work' query from the kickoff
doc, as a standalone tool.
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"

NON_GAME_PREFIXES = ("MarioSt/", "dolsdk2004/", "musyx/")

# Regex to find function definitions in a .c file. Matches common C
# function definition patterns. Used for flavor detection.
FUNC_DEF_RE = re.compile(
    r"^[a-zA-Z_][a-zA-Z0-9_]*\s*\*?\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\([^;]*\)\s*\{",
    re.MULTILINE,
)


def resolve_source_path(unit_name: str) -> Path | None:
    """Map a unit name to its source .c file path.

    TTYD units are named like:
      - "MarioSt/main" or "MarioSt/sequence/seq_game" -> src/main.c, src/sequence/seq_game.c
      - "rels/aji/aji_07" -> rels/aji/src/aji_07.c
    """
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        candidate = REPO / "src" / f"{rel}.c"
        if candidate.exists():
            return candidate
    elif parts[0] == "rels":
        # rels/<area>/<unit> -> rels/<area>/src/<unit>.c
        if len(parts) >= 3:
            area, unit = parts[1], parts[2]
            candidate = REPO / "rels" / area / "src" / f"{unit}.c"
            if candidate.exists():
                return candidate
    return None


def detect_flavor(unit_name: str, asm_path: Path) -> str:
    """Return 'A' (missing from src) or 'B' (present but unmatched)."""
    src = resolve_source_path(unit_name)
    if src is None or not src.exists():
        return "A"

    try:
        source_text = src.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return "B"

    # Count function definitions in the source
    src_fn_defs = FUNC_DEF_RE.findall(source_text)
    src_fn_names = set(m.group(0).split("(")[0].split()[-1] for m in FUNC_DEF_RE.finditer(source_text))

    # Count function symbols in the asm (.fn lines)
    if not asm_path.exists():
        return "A"
    try:
        asm_text = asm_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return "A"
    asm_fn_names = set()
    for line in asm_text.splitlines():
        line = line.strip()
        if line.startswith(".fn "):
            parts = line.split()
            if len(parts) >= 2:
                asm_fn_names.add(parts[1].rstrip(","))

    # If source has 0 function definitions and asm has functions, definitely flavor A
    if not src_fn_names and asm_fn_names:
        return "A"

    # If source has many fn defs but they're stubs (only declarations, no bodies),
    # that's still flavor A. For a quick check: if any source function name appears
    # in the asm, this is likely flavor B.
    if src_fn_names & asm_fn_names:
        return "B"

    # Source has bodies but they don't match asm names -> flavor A
    return "A"


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--max", type=int, default=None, help="max rows")
    p.add_argument("--min-fn", type=int, default=1, help="minimum functions per unit (default 1)")
    p.add_argument("--max-fn", type=int, default=None, help="maximum functions per unit")
    p.add_argument("--game-only", action="store_true", help="exclude SDK/MarioSt/musyx units")
    p.add_argument("--flavor", choices=("A", "B"), default=None, help="filter to one flavor")
    p.add_argument("--csv", action="store_true", help="emit CSV")
    p.add_argument("--version", default=DEFAULT_VERSION, help="build version (default G8MJ01)")
    p.add_argument("--no-flavor-check", action="store_true",
                   help="skip flavor detection (faster, treats all 0% as flavor A)")
    args = p.parse_args()

    build_dir = REPO / "build" / args.version
    report_path = build_dir / "report.json"
    if not report_path.is_file():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())
    measures = report.get("measures", {})

    print(f"project: {measures.get('fuzzy_match_percent', 0):.1f}% fuzzy | "
          f"{measures.get('matched_functions', 0)}/{measures.get('total_functions', 0)} fns "
          f"({measures.get('matched_functions_percent', 0):.1f}%) | "
          f"{measures.get('matched_code_percent', 0):.1f}% bytes")
    print()

    rows: list[dict] = []
    for unit in report["units"]:
        m = unit.get("measures", {})
        tf = m.get("total_functions", 0)
        fp = m.get("fuzzy_match_percent", 0) or 0
        if tf == 0 or fp > 0:
            continue
        if args.game_only and any(unit["name"].startswith(pre) for pre in NON_GAME_PREFIXES):
            continue
        if tf < args.min_fn:
            continue
        if args.max_fn is not None and tf > args.max_fn:
            continue
        total_code = m.get("total_code", 0)

        if args.no_flavor_check:
            flavor = "A"
        else:
            # Find asm path: build/G8MJ01/asm/<unit_name>.s
            asm_path = build_dir / "asm" / f"{unit['name']}.s"
            flavor = detect_flavor(unit["name"], asm_path)
            # TTYD paths use backslashes on Windows; need to also check alt path
            if flavor == "A" and not asm_path.exists():
                asm_path_alt = build_dir / "asm" / f"{unit['name'].replace('/', os_sep())}.s"
                flavor = detect_flavor(unit["name"], asm_path_alt)

        if args.flavor and flavor != args.flavor:
            continue

        rows.append({
            "fn_count": tf,
            "code_size": total_code,
            "name": unit["name"],
            "flavor": flavor,
            "matched_fns": m.get("matched_functions", 0),
        })

    # Sort by function count (smallest first = quickest fresh pools)
    rows.sort(key=lambda r: r["fn_count"])

    if args.max:
        rows = rows[: args.max]

    if not rows:
        print("# no 0% units match the given filters", file=sys.stderr)
        return

    if args.csv:
        w = csv.writer(sys.stdout)
        w.writerow(["flavor", "fn_count", "code_size", "unit", "matched_fns"])
        for r in rows:
            w.writerow([r["flavor"], r["fn_count"], r["code_size"], r["name"], r["matched_fns"]])
        return

    n_a = sum(1 for r in rows if r["flavor"] == "A")
    n_b = sum(1 for r in rows if r["flavor"] == "B")
    print(f"{len(rows)} fully-untouched (0%) units "
          f"(flavor A: {n_a} missing-from-src, flavor B: {n_b} present-but-unmatched):")
    print(f"  {'FN':>3}  {'CODE':>6}  {'M':>3}  {'FLAVOR':<7}  {'UNIT'}")
    for r in rows:
        print(f"  {r['fn_count']:>3}  {r['code_size']:>6}  {r['matched_fns']:>3}  "
              f"{r['flavor']:<7}  {r['name']}")


def os_sep() -> str:
    """Return the OS-specific path separator."""
    import os
    return os.sep


if __name__ == "__main__":
    main()
