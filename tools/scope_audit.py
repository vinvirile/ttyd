#!/usr/bin/env python3
"""scope_audit.py - audit symbol scope mismatches between source and binary.

For a TTYD .c file to be marked as 'Matching' in configure.py, the symbol
scopes (static vs global) MUST match what's in config/G8MJ01/symbols.txt.
If symbols.txt marks a symbol as scope:local, the source must declare it
static (or the linker will fail to resolve). Conversely, scope:global
symbols must NOT be static.

This tool scans all source files and compares declared scopes against
symbols.txt. Reports:
  - Symbols in source but not in symbols.txt (no symbol in binary)
  - Symbols where source scope disagrees with symbols.txt scope
  - Symbols marked scope:local in symbols.txt but referenced as extern
    from another file (these are "static-but-referenced" issues)

Usage:
  python tools/scope_audit.py                        # full audit
  python tools/scope_audit.py <unit_name>            # audit one unit
  python tools/scope_audit.py --extern-refs          # find scope:local symbols referenced as extern
  python tools/scope_audit.py --missing              # find symbols in source but not in binary
  python tools/scope_audit.py --json                 # emit JSON

The output is sorted by severity. The most important class is:
  'SCOPE_LOCAL_REFERENCED' - a scope:local symbol referenced from another
  file, which will fail to link when the file is marked Matching.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"
DEFAULT_CONFIG = "G8MJ01"

# Regex to parse symbols.txt entries
SYMBOL_RE = re.compile(
    r"^(?P<name>\S+)\s*=\s*(?P<section>\S+):0x(?P<address>[0-9A-Fa-f]+);"
    r"(?:\s*//\s*(?P<meta>.*))?$"
)

# Regex to find function/variable declarations in C source
# Matches: [static] [const] type name (args);
# OR:      static type name = ...;
# OR:      type name[count] = ...;
DECL_RE = re.compile(
    r"^(?P<prefix>(?:static\s+|extern\s+)*)"
    r"(?P<type>(?:const\s+)?[a-zA-Z_][a-zA-Z0-9_\s\*]*)"
    r"\b(?P<name>[a-zA-Z_][a-zA-Z0-9_]*)\s*"
    r"(?:\[[^\]]*\])?\s*"
    r"(?:=\s*[^;]+?)?\s*;",
    re.MULTILINE,
)

# File path patterns to find source files
SRC_DIRS = [REPO / "src", REPO / "rels"]


def find_source_for_symbol(symbol: str, symbols: dict) -> str | None:
    """Find the source file that defines a given symbol by searching."""
    for src_root in SRC_DIRS:
        for c_file in src_root.rglob("*.c"):
            try:
                text = c_file.read_text(encoding="utf-8", errors="replace")
            except Exception:
                continue
            # Look for the symbol declared as global (not static) at file scope
            for m in DECL_RE.finditer(text):
                if m.group("name") == symbol and "static" not in m.group("prefix"):
                    return str(c_file.relative_to(REPO))
    return None


def load_symbols(symbols_path: Path) -> dict[str, dict]:
    """Load symbols.txt and return a dict of name -> {section, address, meta}."""
    out: dict[str, dict] = {}
    for line in symbols_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = SYMBOL_RE.match(line.strip())
        if m is None:
            continue
        meta = m.group("meta") or ""
        scope = "global"
        for tok in meta.split():
            if tok.startswith("scope:"):
                scope = tok.split(":", 1)[1]
        out[m.group("name")] = {
            "section": m.group("section"),
            "address": int(m.group("address"), 16),
            "meta": meta,
            "scope": scope,
        }
    return out


def find_extern_refs(symbol: str) -> list[Path]:
    """Find files that reference this symbol as extern (i.e. not defining it)."""
    results: list[Path] = []
    for src_root in SRC_DIRS:
        for c_file in src_root.rglob("*.c"):
            try:
                text = c_file.read_text(encoding="utf-8", errors="replace")
            except Exception:
                continue
            # Look for `extern` declarations of this symbol
            for m in DECL_RE.finditer(text):
                if m.group("name") == symbol and "extern" in m.group("prefix"):
                    results.append(c_file)
                    break
    return results


def audit_unit(unit_name: str, symbols: dict[str, dict]) -> list[dict]:
    """Audit scope mismatches for one unit. Returns list of issue dicts."""
    issues: list[dict] = []
    parts = unit_name.split("/")
    if parts[0] == "MarioSt":
        rel = "/".join(parts[1:]) if len(parts) > 1 else parts[0]
        src_path = REPO / "src" / f"{rel}.c"
    elif parts[0] == "rels":
        if len(parts) >= 3:
            src_path = REPO / "rels" / parts[1] / "src" / f"{parts[2]}.c"
        else:
            return []
    else:
        return []
    if not src_path.exists():
        return []

    try:
        text = src_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return []

    for m in DECL_RE.finditer(text):
        name = m.group("name")
        prefix = m.group("prefix")
        line_no = text[:m.start()].count("\n") + 1

        sym_info = symbols.get(name)
        if sym_info is None:
            # Symbol not in binary - could be from SDK or implicit. Skip.
            continue

        source_scope = "static" if "static" in prefix else "global"
        binary_scope = sym_info["scope"]

        if source_scope == binary_scope:
            continue

        if binary_scope == "local" and source_scope == "global":
            # Source declares as global, but binary has it as local. Linker will fail.
            issues.append({
                "kind": "SCOPE_GLOBAL_DECLARED_LOCAL_BINARY",
                "unit": unit_name,
                "symbol": name,
                "source_scope": source_scope,
                "binary_scope": binary_scope,
                "line": line_no,
                "severity": "high",
            })
        elif binary_scope == "global" and source_scope == "static":
            # Source declares as static, but binary has it as global. Less severe.
            issues.append({
                "kind": "SCOPE_LOCAL_DECLARED_GLOBAL_BINARY",
                "unit": unit_name,
                "symbol": name,
                "source_scope": source_scope,
                "binary_scope": binary_scope,
                "line": line_no,
                "severity": "medium",
            })

    return issues


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("unit", nargs="?", help="unit name to audit (e.g. MarioSt/manager/dvdmgr)")
    p.add_argument("--extern-refs", action="store_true",
                   help="find scope:local symbols referenced as extern from another file")
    p.add_argument("--missing", action="store_true",
                   help="find symbols declared in source but missing from symbols.txt")
    p.add_argument("--version", default=DEFAULT_VERSION, help="build version (default G8MJ01)")
    p.add_argument("--json", action="store_true", help="emit JSON")
    p.add_argument("--max", type=int, default=200, help="max issues to print")
    args = p.parse_args()

    symbols_path = REPO / "config" / args.version / "symbols.txt"
    if not symbols_path.exists():
        sys.exit(f"missing {symbols_path}")
    symbols = load_symbols(symbols_path)

    all_issues: list[dict] = []

    if args.extern_refs:
        # Find scope:local symbols that are referenced as extern from another file
        # First, build a map of source files to their defined symbols
        local_syms = {n: s for n, s in symbols.items() if s["scope"] == "local"}
        for name, sym in local_syms.items():
            refs = find_extern_refs(name)
            for ref in refs:
                all_issues.append({
                    "kind": "SCOPE_LOCAL_REFERENCED",
                    "symbol": name,
                    "section": sym["section"],
                    "address": f"0x{sym['address']:08X}",
                    "extern_decl_in": str(ref.relative_to(REPO)),
                    "severity": "high",
                })
    elif args.unit:
        all_issues.extend(audit_unit(args.unit, symbols))
    else:
        # Full audit: scan all source files
        for src_root in SRC_DIRS:
            for c_file in src_root.rglob("*.c"):
                rel = c_file.relative_to(REPO).as_posix()
                # Map back to unit name
                if rel.startswith("src/"):
                    unit_name = "MarioSt/" + rel[len("src/"):-2]  # strip .c
                elif rel.startswith("rels/"):
                    parts = rel.split("/")
                    if len(parts) >= 4 and parts[2] == "src":
                        unit_name = "rels/" + parts[1] + "/" + parts[3][:-2]
                    else:
                        continue
                else:
                    continue
                all_issues.extend(audit_unit(unit_name, symbols))

    # Sort by severity (high first)
    severity_order = {"high": 0, "medium": 1, "low": 2}
    all_issues.sort(key=lambda x: severity_order.get(x.get("severity", "low"), 3))

    if args.max:
        all_issues = all_issues[: args.max]

    if args.json:
        print(json.dumps(all_issues, indent=2))
        return

    n_high = sum(1 for i in all_issues if i.get("severity") == "high")
    n_med = sum(1 for i in all_issues if i.get("severity") == "medium")
    print(f"Found {len(all_issues)} scope issues "
          f"({n_high} high, {n_med} medium):")
    for i in all_issues:
        if i.get("kind") == "SCOPE_LOCAL_REFERENCED":
            print(f"  HIGH: {i['symbol']} (scope:local in {i['section']}@{i['address']}) "
                  f"referenced as extern in {i['extern_decl_in']}")
        elif i.get("kind") == "SCOPE_GLOBAL_DECLARED_LOCAL_BINARY":
            print(f"  HIGH: {i['unit']}:{i['line']} - {i['symbol']} declared {i['source_scope']} "
                  f"but binary has scope:{i['binary_scope']}")
        elif i.get("kind") == "SCOPE_LOCAL_DECLARED_GLOBAL_BINARY":
            print(f"  MED:  {i['unit']}:{i['line']} - {i['symbol']} declared {i['source_scope']} "
                  f"but binary has scope:{i['binary_scope']}")


if __name__ == "__main__":
    main()
