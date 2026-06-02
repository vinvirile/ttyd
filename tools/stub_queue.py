#!/usr/bin/env python3
"""stub_queue.py - rank undecompiled functions by quick-win likelihood.

Reads build/G8MJ01/report.json and emits a ranked queue of stub functions
(those still at 0% fuzzy match - i.e. either missing from source or completely
non-matching). Bias is toward fast wins: small size + unit already has SOME
matches (evidence that source-set is sane) + tractable shape.

Usage:
  python tools/stub_queue.py                          # full ranked list
  python tools/stub_queue.py --max 50                 # top 50
  python tools/stub_queue.py --max-size 200           # only stubs <= 200 bytes
  python tools/stub_queue.py --unit evtmgr            # stubs in one unit
  python tools/stub_queue.py --aligned-only           # skip units with 0 matched functions
  python tools/stub_queue.py --game-only              # exclude SDK/MarioSt/musyx
  python tools/stub_queue.py --csv > queue.csv

Tip: 'aligned-only' uses the simple proxy `unit has >= 1 matched function`.
For a stricter drift filter, look at the function_order_check tool.

NOTE: This tool is TTYD-tailored. TTYD doesn't use Ghidra placeholders like
fn_XXXXXXXX - instead, 0% units simply have no source file or the function
bodies are entirely fabricated placeholders. Use the --unit filter with
m2c-generated drafts to find candidate work.
"""
from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BUILD = REPO / "build" / "G8MJ01"
DEFAULT_VERSION = "G8MJ01"

# Library prefixes that should be excluded from "game code" triage
NON_GAME_PREFIXES = ("MarioSt/", "dolsdk2004/", "musyx/")


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--max", type=int, default=100, help="max rows (default 100)")
    p.add_argument("--max-size", type=int, default=None, help="only stubs <= N bytes")
    p.add_argument("--min-size", type=int, default=12, help="only stubs >= N bytes (default 12)")
    p.add_argument("--unit", default=None, help="filter by unit substring")
    p.add_argument("--aligned-only", action="store_true", help="skip units with 0 matched functions")
    p.add_argument("--game-only", action="store_true", help="exclude SDK/MarioSt/musyx units")
    p.add_argument("--csv", action="store_true", help="emit CSV format")
    p.add_argument("--version", default=DEFAULT_VERSION, help="build version (default G8MJ01)")
    args = p.parse_args()

    build_dir = REPO / "build" / args.version
    report_path = build_dir / "report.json"
    if not report_path.is_file():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())

    rows: list[dict] = []
    for unit in report["units"]:
        meta = unit.get("metadata", {})
        if meta.get("auto_generated"):
            continue
        name = unit["name"]
        if "unknown/autos" in name:
            continue
        if args.unit and args.unit not in name:
            continue
        if args.game_only and any(name.startswith(pre) for pre in NON_GAME_PREFIXES):
            continue
        measures = unit.get("measures", {})
        matched_fns = measures.get("matched_functions", 0)
        total_fns = measures.get("total_functions", 0)
        if args.aligned_only and matched_fns == 0:
            continue
        for fn in unit.get("functions", []):
            fn_match = fn.get("fuzzy_match_percent", 0) or 0
            if fn_match > 0:
                continue  # already partially or fully matched
            size_str = fn.get("size", "0")
            try:
                size = int(size_str)
            except (ValueError, TypeError):
                size = 0
            if args.min_size and size < args.min_size:
                continue
            if args.max_size and size > args.max_size:
                continue
            # Score: smaller is better, and the unit having existing matches signals tractability.
            tractability_bonus = matched_fns / max(total_fns, 1)
            # Score is "expected effort": small size + high tractability = low score.
            score = size * (1.0 - 0.5 * tractability_bonus)
            rows.append({
                "score": score,
                "size": size,
                "fn": fn["name"],
                "unit": name,
                "matched_fns": matched_fns,
                "total_fns": total_fns,
                "matched_pct": measures.get("matched_code_percent", 0.0) or 0.0,
                "vaddr": fn.get("metadata", {}).get("virtual_address", ""),
            })
    rows.sort(key=lambda r: r["score"])
    rows = rows[: args.max]
    if not rows:
        print("# no stubs match the given filters", file=sys.stderr)
        return
    if args.csv:
        w = csv.writer(sys.stdout)
        w.writerow(["score", "size", "fn", "unit", "matched_fns", "total_fns", "unit_matched_pct", "vaddr"])
        for r in rows:
            w.writerow([f"{r['score']:.0f}", r["size"], r["fn"], r["unit"], r["matched_fns"], r["total_fns"],
                        f"{r['matched_pct']:.2f}", r["vaddr"]])
        return
    print(f"{'SIZE':>5} {'SCORE':>6}  {'UNIT':<46} {'MATCHED/TOTAL':>13}  PCT     FN  (vaddr)")
    for r in rows:
        vaddr = r["vaddr"]
        if vaddr and str(vaddr).isdigit():
            vaddr = f"0x{int(vaddr):08X}"
        ratio = f"{r['matched_fns']}/{r['total_fns']}"
        print(f"{r['size']:>5} {r['score']:>6.0f}  {r['unit']:<46} {ratio:>13}  {r['matched_pct']:5.1f}  {r['fn']}  ({vaddr})")


if __name__ == "__main__":
    main()
