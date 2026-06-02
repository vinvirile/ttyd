#!/usr/bin/env python3
"""find_function.py - find which unit(s) contain a given function symbol.

Searches config/G8MJ01/symbols.txt (and REL symbols files) for a function
symbol and reports:
  - Which unit it lives in (i.e. which .o file)
  - Its address, size, scope
  - Whether the .c file exists for that unit
  - Fuzzy match % of the function (from report.json)

Useful for: "I see a call to `mapSetBg` - which TU defines it?" or
"where is `marioMain` defined?"

Usage:
  python tools/find_function.py <symbol>
  python tools/find_function.py <symbol> --json
  python tools/find_function.py <symbol> --rel <area>   # search only in one REL
  python tools/find_function.py <symbol> --fuzzy         # also show match status

The function name is searched in the config/<version>/symbols.txt files.
For REL functions, also check config/<version>/<area>/symbols.txt.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"

SYMBOL_RE = re.compile(
    r"^(?P<name>\S+)\s*=\s*(?P<section>\S+):0x(?P<address>[0-9A-Fa-f]+);"
    r"(?:\s*//\s*(?P<meta>.*))?$"
)


def find_function_in_symbols(symbols_path: Path, name: str) -> list[dict]:
    """Find a function symbol in a symbols.txt file."""
    results = []
    for line in symbols_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = SYMBOL_RE.match(line.strip())
        if m is None or m.group("name") != name:
            continue
        meta = m.group("meta") or ""
        type_ = "?"
        scope = "global"
        for tok in meta.split():
            if tok.startswith("type:"):
                type_ = tok.split(":", 1)[1]
            elif tok.startswith("scope:"):
                scope = tok.split(":", 1)[1]
        size_m = re.search(r"\bsize:0x([0-9A-Fa-f]+)\b", meta)
        size = int(size_m.group(1), 16) if size_m else None
        results.append({
            "name": m.group("name"),
            "section": m.group("section"),
            "address": int(m.group("address"), 16),
            "type": type_,
            "scope": scope,
            "size": size,
            "meta": meta,
        })
    return results


def infer_unit_from_address(version: str, address: int, section: str) -> str:
    """Infer the unit name (for report.json) from an address.

    TTYD's address space is split between the DOL and RELs. We can tell from
    the section name (DOL is .text, RELs are also .text) and from reading
    config/G8MJ01/splits.txt to find which TU owns the address.
    """
    splits_path = REPO / "config" / version / "splits.txt"
    if not splits_path.exists():
        return "?"
    for line in splits_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        # Format: section_name = start_addr, end_addr, unit_name
        m = re.match(r"^(\S+)\s*=\s*0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+),\s*(\S+)", line)
        if m is None:
            continue
        sec, start, end, unit = m.groups()
        if sec != section:
            continue
        start, end = int(start, 16), int(end, 16)
        if start <= address < end:
            return unit
    return "?"


def find_fuzzy_match(report: dict, fn_name: str) -> tuple[str | None, float]:
    """Find the fuzzy match % for a function across all units."""
    for u in report["units"]:
        for fn in u.get("functions", []):
            if fn["name"] == fn_name:
                return u["name"], fn.get("fuzzy_match_percent", 0) or 0
    return None, 0.0


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("symbol", help="function symbol name to find")
    p.add_argument("--version", default=DEFAULT_VERSION)
    p.add_argument("--rel", default=None, help="restrict to one REL area")
    p.add_argument("--include-rels", action="store_true", help="search all REL symbols.txt")
    p.add_argument("--fuzzy", action="store_true", help="also show fuzzy match status")
    p.add_argument("--json", action="store_true", help="emit JSON")
    args = p.parse_args()

    config_dir = REPO / "config" / args.version
    if not config_dir.exists():
        sys.exit(f"config dir not found: {config_dir}")

    # Determine which symbols files to search
    paths: list[Path] = []
    if args.rel:
        rel_path = config_dir / args.rel / "symbols.txt"
        if not rel_path.exists():
            sys.exit(f"REL symbols not found: {rel_path}")
        paths.append(rel_path)
    else:
        dol_path = config_dir / "symbols.txt"
        if dol_path.exists():
            paths.append(dol_path)
        if args.include_rels:
            for sub in config_dir.iterdir():
                if sub.is_dir() and (sub / "symbols.txt").exists():
                    paths.append(sub / "symbols.txt")

    # Search
    matches: list[dict] = []
    for p_ in paths:
        for sym in find_function_in_symbols(p_, args.symbol):
            # Determine the source file path
            sym["source_file"] = str(p_.relative_to(REPO))
            # Try to infer unit name
            unit = infer_unit_from_address(args.version, sym["address"], sym["section"])
            sym["unit"] = unit
            matches.append(sym)

    # Filter to functions (type:func, type:function, or no type specified)
    fn_matches = [m for m in matches if m["type"] in ("func", "function", "?")]
    other_matches = [m for m in matches if m["type"] not in ("func", "function", "?")]

    if args.fuzzy:
        report_path = REPO / "build" / args.version / "report.json"
        if report_path.exists():
            report = json.loads(report_path.read_text())
            for m in fn_matches:
                _, pct = find_fuzzy_match(report, m["name"])
                m["fuzzy_match_percent"] = pct

    if args.json:
        print(json.dumps({"functions": fn_matches, "other_symbols": other_matches}, indent=2))
        return

    if not matches:
        sys.exit(f"symbol '{args.symbol}' not found")

    print(f"=== Search results for '{args.symbol}' ===")
    print()
    if fn_matches:
        print(f"  Functions ({len(fn_matches)}):")
        print(f"    {'ADDR':<12} {'SIZE':>8}  {'SCOPE':<8}  {'UNIT'}")
        for m in fn_matches:
            size = f"0x{m['size']:X}" if m["size"] else "?"
            fuzzy = ""
            if args.fuzzy and "fuzzy_match_percent" in m:
                fuzzy = f"  [{m['fuzzy_match_percent']:.1f}% match]"
            print(f"    0x{m['address']:08X}  {size:>8}  {m['scope']:<8}  {m['unit']}{fuzzy}")
    if other_matches:
        print(f"  Other symbols ({len(other_matches)}):")
        for m in other_matches:
            print(f"    0x{m['address']:08X}  type:{m['type']:<8}  {m['name']}")


if __name__ == "__main__":
    main()
