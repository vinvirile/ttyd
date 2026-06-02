#!/usr/bin/env python3
"""symbol_neighbors.py - find symbols near an address or symbol name.

For understanding context around a known symbol - useful when:
  - Disassembling a function and need to know what calls it
  - Looking for the function that owns an unknown address
  - Understanding which TU a stray address belongs to

Searches both config/G8MJ01/symbols.txt (DOL + all RELs) for the closest
symbols to a given address or name.

Usage:
  python tools/symbol_neighbors.py <query>            # 5 neighbors before/after
  python tools/symbol_neighbors.py <query> -n 10      # 10 neighbors
  python tools/symbol_neighbors.py 0x80043100         # hex address
  python tools/symbol_neighbors.py swGet              # symbol name
  python tools/symbol_neighbors.py swGet --rel mri    # restrict to one REL
  python tools/symbol_neighbors.py swGet --type func  # filter by type
"""
from __future__ import annotations

import argparse
import bisect
import re
import sys
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_VERSION = "G8MJ01"

SYMBOL_RE = re.compile(
    r"^(?P<name>\S+)\s*=\s*(?P<section>\S+):0x(?P<address>[0-9A-Fa-f]+);"
    r"(?:\s*//\s*(?P<meta>.*))?$"
)


@dataclass(frozen=True)
class Symbol:
    name: str
    section: str
    address: int
    type: str
    scope: str
    meta: str

    @property
    def size(self) -> int | None:
        m = re.search(r"\bsize:0x([0-9A-Fa-f]+)\b", self.meta)
        return int(m.group(1), 16) if m else None


def load_symbols(symbols_path: Path) -> list[Symbol]:
    symbols: list[Symbol] = []
    for line in symbols_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = SYMBOL_RE.match(line.strip())
        if m is None:
            continue
        meta = m.group("meta") or ""
        type_ = "?"
        scope = "global"
        for tok in meta.split():
            if tok.startswith("type:"):
                type_ = tok.split(":", 1)[1]
            elif tok.startswith("scope:"):
                scope = tok.split(":", 1)[1]
        symbols.append(
            Symbol(
                name=m.group("name"),
                section=m.group("section"),
                address=int(m.group("address"), 16),
                type=type_,
                scope=scope,
                meta=meta,
            )
        )
    symbols.sort(key=lambda s: (s.address, s.name))
    return symbols


def parse_query(query: str) -> int | str:
    q = query.strip()
    if q.lower().startswith("0x"):
        return int(q, 16)
    if q.isdigit():
        return int(q, 16)
    return q


def find_center(symbols: list[Symbol], query: int | str) -> int:
    if isinstance(query, str):
        for i, s in enumerate(symbols):
            if s.name == query:
                return i
        # Try partial match
        partials = [i for i, s in enumerate(symbols) if query in s.name]
        if len(partials) == 1:
            return partials[0]
        if partials:
            print(f"# {len(partials)} partial matches for '{query}', showing nearest",
                  file=sys.stderr)
            return partials[0]
        raise SystemExit(f"Symbol not found: {query}")

    addresses = [s.address for s in symbols]
    index = bisect.bisect_left(addresses, query)
    if index >= len(symbols):
        return len(symbols) - 1
    if symbols[index].address == query or index == 0:
        return index
    prev_dist = query - symbols[index - 1].address
    next_dist = symbols[index].address - query
    return index - 1 if prev_dist <= next_dist else index


def format_symbol(symbol: Symbol) -> str:
    size = f" size=0x{symbol.size:X}" if symbol.size is not None else ""
    scope = f" scope:{symbol.scope}"
    type_ = f" type:{symbol.type}" if symbol.type != "?" else ""
    return (f"0x{symbol.address:08X} {symbol.section:<10}{type_:<12}{scope:<14} "
            f"{symbol.name}{size}")


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("query", help="Symbol name or hex address (with 0x prefix)")
    p.add_argument("-n", "--neighbors", type=int, default=5,
                   help="number of symbols before and after (default 5)")
    p.add_argument("--version", default=DEFAULT_VERSION, help="version dir (default G8MJ01)")
    p.add_argument("--rel", default=None, help="restrict to one REL module (e.g. mri)")
    p.add_argument("--type", default=None, help="filter by type (e.g. func, label, object)")
    p.add_argument("--scope", default=None, help="filter by scope (global/local/weak)")
    p.add_argument("--include-rels", action="store_true",
                   help="also include symbols from REL modules (slower)")
    args = p.parse_args()

    paths: list[Path] = [REPO / "config" / args.version / "symbols.txt"]
    if args.rel:
        paths.append(REPO / "config" / args.version / args.rel / "symbols.txt")
    elif args.include_rels:
        for sub in (REPO / "config" / args.version).iterdir():
            if sub.is_dir() and (sub / "symbols.txt").exists():
                paths.append(sub / "symbols.txt")

    symbols: list[Symbol] = []
    for p_ in paths:
        if not p_.exists():
            if args.rel and p_ == paths[-1]:
                sys.exit(f"REL symbols file not found: {p_}")
            continue
        # Tag the symbol with its source file so the user can tell which is which
        loaded = load_symbols(p_)
        # Skip if not in the requested REL filter
        if args.rel and str(p_).find(f"/{args.rel}/") < 0:
            continue
        symbols.extend(loaded)

    if not symbols:
        sys.exit("no symbols loaded")

    # Optional filters
    if args.type:
        symbols = [s for s in symbols if s.type == args.type]
    if args.scope:
        symbols = [s for s in symbols if s.scope == args.scope]

    center = find_center(symbols, parse_query(args.query))
    start = max(0, center - args.neighbors)
    end = min(len(symbols), center + args.neighbors + 1)

    for i in range(start, end):
        prefix = ">>>" if i == center else "   "
        print(f"{prefix} {format_symbol(symbols[i])}")


if __name__ == "__main__":
    main()
