#!/usr/bin/env python3
"""function_order_check.py - verify function order for -inline deferred.

TTYD uses MWCC's -inline deferred flag for DOL game code, which causes the
compiler to REVERSE function emission order. Source order must be the
REVERSE of binary order, or the compiled .o will not match.

This tool:
  1. Reads the binary's symbol order from the original .o (via dtk elf info).
  2. Reads the source's function definition order.
  3. Compares them and reports:
     - Whether they match (in reverse)
     - The expected source order to match the binary
     - Any functions in the binary missing from source
     - Any functions in source missing from the binary

Usage:
  python tools/function_order_check.py <unit_name>     # check one unit
  python tools/function_order_check.py --all-mismatch  # check all mismatching units
  python tools/function_order_check.py --game-only    # skip SDK/MarioSt/musyx
  python tools/function_order_check.py --rel          # REL modules (NORMAL order, not reversed)

How to read dtk output:
  build\\tools\\dtk elf info <obj>.o
  The "Symbol table" section lists symbols in binary order. Functions are
  type:function entries. With -inline deferred, the LAST function in source
  is emitted FIRST in the binary, etc.

For REL modules, the function order is NORMAL (REL uses -O4,s without
-inline deferred). Use --rel to check those.

Example output:
  Unit: MarioSt/driver/swdrv
  Binary has 12 functions:
    1. swInit
    2. swReInit
    ...
    12. swGetCurNo
  Source has 12 functions (in order):
    1. swInit
    2. swReInit
    ...
  EXPECTED SOURCE ORDER (reverse of binary):
    swGetCurNo, swSetCurNo, ..., swInit
  STATUS: MISMATCH - source order does not match (would need to be reversed)
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"

# Function definition pattern - matches common C function definitions including
# the TTYD-specific USER_FUNC expansion `s32(name)(EventEntry*, BOOL) {`.
# Also matches `static TYPE name(args) {` and `TYPE name(args) {` patterns.
FUNC_DEF_RE = re.compile(
    r"^(?:static\s+)?(?:inline\s+)?"
    r"(?:[a-zA-Z_][a-zA-Z0-9_\s\*]*?)"
    r"(?:\(([a-zA-Z_][a-zA-Z0-9_]*)\)|([a-zA-Z_][a-zA-Z0-9_]*))"
    r"\s*\([^;]*?\)\s*\{",
    re.MULTILINE,
)


def _get_fn_name(m: re.Match) -> str:
    return m.group(1) or m.group(2)


def resolve_source_path(unit_name: str) -> Path | None:
    """Map a unit name to its source .c file path."""
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        candidate = REPO / "src" / f"{rel}.c"
        if candidate.exists():
            return candidate
    elif parts[0] == "rels":
        if len(parts) >= 3:
            area, unit = parts[1], parts[2]
            candidate = REPO / "rels" / area / "src" / f"{unit}.c"
            if candidate.exists():
                return candidate
    return None


def get_obj_path(unit_name: str, version: str) -> Path:
    """Resolve a unit name to its original .o file path.

    TTYD layout: build/<version>/obj/<rest_of_name>.o
    REL layout:  build/<version>/<area>/obj/<unit>.o
    """
    if unit_name.startswith("rels/"):
        parts = unit_name.split("/")
        if len(parts) >= 3:
            return REPO / "build" / version / parts[1] / "obj" / f"{parts[2]}.o"
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        return REPO / "build" / version / "obj" / f"{rel}.o"
    return REPO / "build" / version / "obj" / f"{unit_name}.o"


def get_binary_function_order(unit_name: str, version: str, dtk_path: Path) -> list[str]:
    """Use dtk elf info to get the function order from the original .o file.

    Returns function names in binary order (FIRST emitted = index 0).
    """
    obj_path = get_obj_path(unit_name, version)
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

    # Parse dtk output. dtk's "Symbols:" section has a tabular format like:
    #           Section | Address    | Size       | Name
    #             .text | 0x0        | 0x10       | _swByteGet
    #             .text | 0x10       | 0x10       | _swByteSet
    # The order in this table is the binary's symbol order. Function symbols
    # in TTYD's binary typically have a section of .text and the name is a
    # plain C identifier.
    #
    # Strategy: find the Symbols: section, then read lines that look like
    #   <section> | <addr> | <size> | <name>
    # Filter to lines where section is .text (function code) and the name
    # is a valid C identifier. Track order.

    in_symbols = False
    fns: list[str] = []
    for line in result.stdout.splitlines():
        stripped = line.strip()
        if stripped == "Symbols:":
            in_symbols = True
            continue
        if in_symbols:
            # End of symbols section: a line that doesn't have the | separator
            if "|" not in stripped and stripped:
                # Could be the next section header
                in_symbols = False
                continue
        if not in_symbols or "|" not in stripped:
            continue
        # Split on | and parse columns
        parts = [p.strip() for p in stripped.split("|")]
        if len(parts) < 4:
            continue
        section, addr, size, name = parts[0], parts[1], parts[2], parts[3]
        if section != ".text":
            continue
        if not re.match(r"^[a-zA-Z_][a-zA-Z0-9_]*$", name):
            continue
        # Skip compiler-generated symbols
        if name.startswith("__"):
            continue
        fns.append(name)

    return fns


def get_source_function_order(source_path: Path) -> list[str]:
    """Parse the .c file and return function definition names in source order."""
    if not source_path.exists():
        return []
    try:
        text = source_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return []
    return [_get_fn_name(m) for m in FUNC_DEF_RE.finditer(text)]


def check_unit_order(
    unit_name: str,
    version: str,
    dtk_path: Path,
    is_rel: bool = False,
) -> dict:
    """Check function order for one unit. Returns a result dict."""
    source_path = resolve_source_path(unit_name)
    binary_order = get_binary_function_order(unit_name, version, dtk_path)
    source_order = get_source_function_order(source_path) if source_path else []

    if not binary_order:
        return {
            "unit": unit_name,
            "source": str(source_path) if source_path else None,
            "binary_order": [],
            "source_order": source_order,
            "expected_source_order": [],
            "status": "NO_BINARY",
            "is_rel": is_rel,
        }

    if is_rel:
        # REL modules have NORMAL order (no -inline deferred)
        expected = binary_order
        reversed_mode = False
    else:
        # DOL has REVERSED order (-inline deferred)
        expected = list(reversed(binary_order))
        reversed_mode = True

    return {
        "unit": unit_name,
        "source": str(source_path) if source_path else None,
        "binary_order": binary_order,
        "source_order": source_order,
        "expected_source_order": expected,
        "reversed": reversed_mode,
        "is_rel": is_rel,
        "status": "OK" if source_order == expected else "MISMATCH",
    }


def print_check_result(result: dict) -> None:
    unit = result["unit"]
    print(f"=== {unit} ===")
    print(f"  Source: {result['source'] or '(missing)'}")
    print(f"  Binary has {len(result['binary_order'])} functions:")
    for i, fn in enumerate(result["binary_order"], 1):
        print(f"    {i:>3}. {fn}")
    if not result["source_order"]:
        print("  (no source functions found)")
    else:
        print(f"  Source has {len(result['source_order'])} functions (in order):")
        for i, fn in enumerate(result["source_order"][:5], 1):
            print(f"    {i:>3}. {fn}")
        if len(result["source_order"]) > 5:
            print(f"    ... and {len(result['source_order']) - 5} more")
    if result["expected_source_order"]:
        print(f"  Expected source order (binary order {'REVERSED' if result['reversed'] else 'NORMAL'}):")
        for i, fn in enumerate(result["expected_source_order"][:5], 1):
            print(f"    {i:>3}. {fn}")
        if len(result["expected_source_order"]) > 5:
            print(f"    ... and {len(result['expected_source_order']) - 5} more")
    print(f"  STATUS: {result['status']}")
    if result["status"] == "MISMATCH":
        # Show the diff
        expected = set(result["expected_source_order"])
        actual = set(result["source_order"])
        missing = expected - actual
        extra = actual - expected
        if missing:
            print(f"    Missing from source: {sorted(missing)}")
        if extra:
            print(f"    In source but not in binary: {sorted(extra)}")
    print()


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("unit", nargs="?", help="unit name to check (e.g. MarioSt/driver/swdrv)")
    p.add_argument("--all-mismatch", action="store_true",
                   help="scan all units in report.json and report mismatches")
    p.add_argument("--rel", action="store_true",
                   help="treat as REL module (NORMAL order, not reversed)")
    p.add_argument("--version", default=DEFAULT_VERSION, help="build version (default G8MJ01)")
    p.add_argument("--dtk", default=None, help="path to dtk executable (default: build/tools/dtk)")
    args = p.parse_args()

    if not args.unit and not args.all_mismatch:
        p.error("either a unit name or --all-mismatch is required")

    dtk_path = Path(args.dtk) if args.dtk else (REPO / "build" / "tools" / ("dtk.exe" if sys.platform == "win32" else "dtk"))
    if not dtk_path.exists():
        sys.exit(f"dtk not found at {dtk_path}")

    if args.unit:
        is_rel = args.rel or args.unit.startswith("rels/")
        result = check_unit_order(args.unit, args.version, dtk_path, is_rel=is_rel)
        print_check_result(result)
        sys.exit(0 if result["status"] == "OK" else 1)

    # --all-mismatch mode
    report_path = REPO / "build" / args.version / "report.json"
    if not report_path.exists():
        sys.exit(f"missing {report_path}; run `ninja build/{args.version}/report.json` first")
    report = json.loads(report_path.read_text())

    mismatches: list[dict] = []
    for unit in report["units"]:
        name = unit["name"]
        m = unit.get("measures", {})
        # Skip auto-generated and unknown units
        if unit.get("metadata", {}).get("auto_generated"):
            continue
        if "unknown/autos" in name:
            continue
        # Skip units with 0 functions (no asm to check)
        if m.get("total_functions", 0) == 0:
            continue
        is_rel = name.startswith("rels/")
        result = check_unit_order(name, args.version, dtk_path, is_rel=is_rel)
        if result["status"] == "MISMATCH":
            mismatches.append(result)

    print(f"{len(mismatches)} unit(s) with function order mismatches:")
    for r in mismatches:
        n_bin = len(r["binary_order"])
        n_src = len(r["source_order"])
        marker = "  " if n_bin == n_src else "**"
        print(f"{marker} {r['unit']:<50}  binary={n_bin:>3}  source={n_src:>3}  "
              f"{'(REL)' if r['is_rel'] else '(DOL)'}")


if __name__ == "__main__":
    main()
