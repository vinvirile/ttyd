# Agent Guide for Paper Mario: The Thousand-Year Door (TTYD) Decompilation

This document describes the structure, conventions, and workflows of this repository to help AI agents contribute effectively. The project is a matching decompilation of **Paper Mario: The Thousand-Year Door** (GameCube, MWCC GC/2.6 C compiler, PowerPC Gekko target).

## Project Goal

Reconstruct the original C source code such that compiling it with the original Metrowerks CodeWarrior compiler produces **byte-identical** object files to the original game binary (`main.dol` and 30+ `.rel` modules). This is called a **matching** decomp.

The long-term goal is to produce a clean, portable codebase that can later be paired with a custom graphics backend for modern platforms. But the immediate priority is matching the original binary.

### Mindset

The work is fundamentally about getting into the heads of the original developers using only the evidence available — the binary, the symbol table, debug strings, and the compiler's behavior. Every assumption must be doubted and verified. Be sceptical of everything: naming guesses, code structure, control flow choices, even whether a function "looks right." Only commit to a decision when the evidence is overwhelming.

When progress stalls because a tough choice must be made and strong evidence is lacking, **leave the code nonmatching and move on**. New evidence often emerges later — from matching a neighboring function, finding a pattern in another TU, or discovering a debug string. Premature commitment to a wrong approach creates technical debt that's harder to undo than a TODO comment.

Truly hard judgment calls — ambiguous code structure, naming disputes, architectural decisions — should be deferred to humans. The ultimate goal is not just a matching binary but **human-readable, modifiable source code**.

## Repository Layout

```
config/G8MJ01/
  config.yml          — decomp-toolkit project config (DOL + all REL modules)
  symbols.txt         — every symbol in the DOL: address, size, type, scope
  splits.txt          — per-TU section address ranges for the DOL
  build.sha1          — SHA1 hash of the target DOL
  <area>/             — per-REL config (e.g. mri/, aaa/, gor/, etc.)
    symbols.txt       — symbols for that REL module
    splits.txt        — splits for that REL module

src/                  — decompiled C source files (DOL code)
include/              — headers for DOL code
rels/                 — REL module source (only modules with active decomp work)
  <area>/src/         — C source for a REL module
  <area>/include/     — headers for a REL module
libs/
  dolsdk2004/         — Dolphin SDK (OS, GX, DVD, PAD, VI, etc.)
  musyx/              — MusyX audio middleware
orig/G8MJ01/          — original game disc image (not committed)
build/G8MJ01/         — build artifacts, compiled objects
  asm/                — dtk-generated assembly files per TU
  obj/                — target (original) object files extracted by dtk
  src/                — recompiled object files from our source
build/compilers/      — retro compilers, downloaded during build
build/tools/          — dtk, objdiff-cli, sjiswrap

tools/
  decompctx.py        — generates decomp.me context files
  project.py          — build system library

configure.py          — main build configuration; lists ALL objects and matching status
objdiff.json          — auto-generated config for objdiff
assets/               — extracted game assets (textures, etc.)
```

## Build & Diff Workflow

Ninja is used for building. Running `ninja` in the root builds the project and reports matching progress.

```
python configure.py   # generate build files (run once, or after config changes)
ninja                 # build and report progress
```

The underlying utility used for splitting the binary is `dtk` (`build/tools/dtk`).

The main diffing tool used by humans is **objdiff** (`encounter/objdiff`). It compares the compiled `.o` from our source against the original `.o` extracted from the DOL/REL, function by function, showing PPC assembly side-by-side. It automatically rebuilds on file changes.

**Important for agents**: The human user typically has objdiff open in the background. It watches source files and **automatically recompiles** whenever a file changes. After editing a source or header file, you do **not** need to manually force rebuilds — just run `ninja` and if it says "no work to do", that's fine; objdiff has already compiled the latest code.

### Using objdiff-cli (agent-friendly diffing)

Since agents cannot use the objdiff GUI, use `objdiff-cli` directly. A wrapper script (`tools/decomp-diff.py`) is planned but not yet available.

Unit names follow the pattern in `objdiff.json`. You can grep `objdiff.json` for a source filename to find its unit name.

```
build/tools/objdiff-cli diff -p build/G8MJ01 <unit_name>
```

To list all units and their status:
```
build/tools/objdiff-cli report -p build/G8MJ01
```

### Using `m2c` for raw draft decompilation

If a local clone of [`m2c`](https://github.com/matt-kempster/m2c) is available, it can produce a **rough first-pass C draft** from dtk-generated assembly files under `build/G8MJ01/asm/<path>.s`.

`m2c` works as an asm-to-draft tool, not as a source-of-truth decompiler. Treat its output as scaffolding for understanding control flow and rough data flow, then verify everything against objdiff and the binary.

```bash
python /path/to/m2c/m2c.py -t ppc -f function_name --globals=none build/G8MJ01/asm/<path>.s
```

- Use `-t ppc` (the right alias for CodeWarrior PowerPC output).
- Use `--globals=none` when you only want the function body draft.
- Find mangled function names from `.fn` lines in the `.s` files.

Important limitations:
- `m2c` output is often rough: inferred placeholder structs, bad field names, missing types, and occasional broken expressions are normal.
- Use `m2c` for orientation only; never trust it over the asm or objdiff.

## Source Organization

Each `.o` file maps 1:1 to a `.c` file. The path is listed in `configure.py` under `config.libs`. Each object has a status:

- `Matching` — our code compiles to byte-identical output. Linked into the final binary.
- `NonMatching` — work in progress. Not linked.
- `Equivalent` — functionally equivalent but not byte-identical. Only linked with `--non-matching`.

### DOL Library Groups

| Directory | Description |
|-----------|-------------|
| `src/battle/` | Battle system (combat, action commands, audience, stage, camera) |
| `src/battle/ac/` | Action command implementations (timing, gauge, stick, button) |
| `src/battle/sac/` | Special action commands (partner super moves) |
| `src/data/` | Static game data (items, mail, maps, NPCs) |
| `src/driver/` | Driver layer: rendering, NPC, map, camera, effect, message, etc. |
| `src/effect/` | Visual effects (GC-native effects) |
| `src/effect/n64/` | Visual effects ported from Paper Mario 64 |
| `src/event/` | Event script command implementations (evt_*.c) |
| `src/manager/` | Manager subsystems (event, font, card, DVD, file, window) |
| `src/mario/` | Mario player code (movement, hit detection, motion, pouch) |
| `src/motion/` | Mario animation/motion data (walk, jump, hammer, damage, etc.) |
| `src/party/` | Party member implementations (Yoshi, Goombella, Koops, etc.) |
| `src/sequence/` | Game sequence management (title, game, battle, load, demo) |
| `src/unit/` | Unit/enemy behavior definitions |
| `src/window/` | UI window system (menus, badge, item, log) |
| `src/sdk/` | Game-specific SDK wrappers (arc, DEMO, texPalette) |

### REL Modules

REL (Relocatable) modules are loaded at runtime for specific game areas/chapters. Each has its own `symbols.txt` and `splits.txt` under `config/G8MJ01/<area>/`. Module names use Japanese abbreviations:

| Module | Area (likely) |
|--------|---------------|
| `mri` | Mushroom Road / Mario's house area |
| `gor` | Rogueport |
| `tik` | Petal Meadows |
| `hei` | Hooktail Castle |
| `nok` | Petalburg |
| `muj` | Twilight Town |
| `tou` | Twilight Trail |
| `usu` | Creepy Steeple |
| `rsh` | Excess Express (train) |
| `aji` | Fahr Outpost |
| `bom` | Bomb-omb Battlefield |
| `dig` | Glitzville |
| `dmo` | Demo/debug |
| `dou` | Pirate's Grotto |
| `eki` | Riverside Station |
| `end` | Ending |
| `gon` | Dragon's Lair / Gloomtail |
| `gra` | Glitzville arena |
| `hom` | Rogueport Sewers |
| `jin` | Poshley Heights |
| `jon` | Poshley Sanctum |
| `kpa` | Bowser's Castle |
| `las` | Palace of Shadow |
| `moo` | The Moon |
| `pik` | Riverside Station |
| `qiz` | Quiz Show |
| `sys` | System / common REL code |
| `tou2` | Twilight Trail (part 2) |
| `win` | Pit of 100 Trials |
| `yuu` | Keelhaul Key |

## Compiler & Flags

- **Game code (DOL)**: Metrowerks CodeWarrior `GC/1.3.2` (produces version string 2.4.2.1)
- **SDK (Dolphin)**: `GC/1.2.5n`
- **MusyX audio**: `GC/1.3.2`
- **REL modules**: `GC/1.3.2`
- **Language**: C (not C++)
- **Exceptions**: off

**Important**: The original game was compiled with GC/1.3.2 (version 2.4.2.1), NOT GC/2.6 (version 2.4.7.1). The configure.py must use GC/1.3.2 for game code to produce matching output. Using the wrong compiler version will cause .comment section differences and potentially different code generation.

### Key flag sets

**DOL game code** (`cflags_static`):
```
-O4,p -fp_contract on -inline auto,deferred -use_lmw_stmw on -rostr -sdata 48 -sdata2 6
```

**REL code** (`cflags_rel`):
```
-O4,s -use_lmw_stmw on -rostr -sdata 0 -sdata2 0
```

**Base flags** (common to all):
```
-nodefaults -proc gekko -align powerpc -enum int -multibyte -char unsigned
-fp hardware -Cpp_exceptions off -pragma "cats off" -O4,p -inline auto
-once -maxerrors 1 -nosyspath
```

### Compiler behavior to be aware of

- MWCC aggressively inlines small functions, especially those defined in headers or marked inline.
- The compiler preserves symbol order strictly — function/variable definition order in the source must match the order in the binary.
- With `-inline deferred` (used for most game code), function emission order is **reversed** relative to source order.
- Dead code elimination is limited — unused `static` variables and functions may still be emitted.
- BSS ordering is affected by include order.
- `#pragma dont_inline on/off` prevents inlining of specific functions when needed for matching.
- `#pragma pool_data on` controls string/float literal pooling.
- `#pragma warn_no_side_effect off` suppresses warnings for code with no side effects.

### Symbol Ordering in ELF Files

The MWCC compiler places symbols in the ELF symbol table in a specific order that differs from the original binary:
- **Static symbols** are placed before **global symbols**
- Within each category, the order may differ from source declaration order
- This causes the `.strtab` (string table) section to differ, which affects the hash

This is a fundamental compiler behavior that cannot be controlled from source code. Files with data sections (variables, arrays) will have different `.strtab` content than the original binary, even if the code and data are byte-identical.

**Impact**: Files with only code (no data sections) can match. Files with data sections cannot match due to symbol ordering differences in `.strtab`.

## Code Conventions

### Naming

- **Function names**: Often Japanese romaji or English. Examples: `marioStInit`, `evtEntry`, `npcEntry`, `mapGetMapName`.
- **Driver functions**: Prefixed with driver name: `mapdrv*`, `camdrv*`, `npcdrv*`, `effdrv*`, `msgdrv*`, etc.
- **Event system**: `evt*` prefix: `evtEntry`, `evtDelete`, `evtSetPri`, `evtGetWork`.
- **Manager functions**: `*mgr*` pattern: `evtmgrInit`, `fontmgrInit`, `cardmgrInit`.
- **Local variables**: `camelCase` (e.g., `priEntryCount`, `slotI`). Or placeholders: `fVar1`, `uVar2`, etc.
- **Function arguments**: `camelCase` or placeholders: `a1`, `a2`, etc.
- **Unknown members**: `unk*` with hex offset: `unk194`, `unk238`.
- **Global work pointer**: `extern GlobalWork* gp;` — the main game state struct.
- **Japanese strings**: Appear in debug strings, NPC names, and data tables.

### Section annotations

Source files use comments to mark data sections:
```c
//.data
static f32 angleABTBL[] = { ... };

//.bss
static EventWork work[2];

//.sdata
s32 evtID = 1;

//.sbss
s32 evtMax;
```

### Style

- Formatting follows `.clang-format` (4-space indent, 120-column, no tabs).
- C style: no C++ features, no `bool` (use `BOOL` from Dolphin SDK).
- `#include` paths: quoted for project headers (`"manager/evtmgr.h"`), angle brackets for SDK (`<dolphin/os.h>`).
- Header guards: `#pragma once`.
- Integer types from Dolphin SDK: `s8`, `s16`, `s32`, `u8`, `u16`, `u32`, `f32`, `f64`.

### Comments & annotations

- `/* "filename" - Description */` — file-level header comment with status.
- `// TODO:` — known issues, suspected inaccuracies, or incomplete understanding.
- `// fabricated` — code not from the original binary, invented to make things compile.
- `// size 0xNNNN` — size annotations on BSS/data declarations.

### Fakematches

A **fakematch** is code that compiles to byte-identical output but is clearly not what the original developers wrote. Fakematches should be **avoided where possible** — the goal is to reconstruct code that a real developer at Intelligent Systems would have plausibly written in ~2004 using C. If a match can only be achieved through obviously artificial code, it's better to leave the function as nonmatching with a TODO comment.

## Key Data Files

### `config/G8MJ01/symbols.txt`

Every symbol in the DOL binary with address, size, type, scope and alignment. Format:
```
symbol_name = .section:0xADDRESS; // type:function size:0xSIZE scope:global
```

Scope is `global`, `local` (static), or `weak` (inline/header-defined). This file has 36,000+ entries.

### `config/G8MJ01/splits.txt`

Maps each source file to its section address ranges, telling dtk how to split the DOL into per-TU objects.

### `config/G8MJ01/config.yml`

Project configuration including DOL hash, symbol/split file paths, and all REL module definitions with their hashes and per-module config paths.

### `build/G8MJ01/asm/<path>.s`

Disassembled `.s` files generated by dtk for every TU. These contain the full assembly including data sections (`.rodata`, `.data`, `.sdata2`, etc.) with literal values — strings, floats, jump tables. Useful for recovering string constants, float literals, and other data symbol contents.

## Architecture & Game Systems

### Event/Script System

The game uses a bytecode-based event scripting system, central to all game logic:

- `evtmgr.c` / `evtmgr.h` — Event entry, scheduling, priority management.
- `evtmgr_cmd.c` / `evtmgr_cmd.h` — Command interpreter (118+ opcodes).
- `evt_*.c` files — Domain-specific event commands (NPC, map, message, camera, etc.).
- Scripts are arrays of `EventCommand` structs with opcodes and arguments.
- Events have priorities, types, labels, loops, switches, and can spawn child/sibling events.
- `UserFunction` callbacks allow C functions to be called from scripts.

### Driver Architecture

The game uses a driver pattern where each subsystem has a `*drv.c` file managing initialization, updates, and rendering:

| Driver | Purpose |
|--------|---------|
| `mapdrv` | Map/field rendering and collision |
| `camdrv` | Camera system |
| `npcdrv` | NPC management and AI |
| `effdrv` | Visual effects |
| `msgdrv` | Message/text display |
| `hitdrv` | Hit detection / collision |
| `dispdrv` | Display list management |
| `animdrv` | Animation playback |
| `mobjdrv` | Map objects (chests, signs, etc.) |
| `swdrv` | Switch/flag management |
| `itemdrv` | Item management |
| `windowdrv` | Window/UI rendering |
| `fadedrv` | Screen fade transitions |
| `shadowdrv` | Shadow rendering |
| `lightdrv` | Lighting |
| `envdrv` | Environment effects |

### Global State

`GlobalWork* gp` is the main game state struct, accessed everywhere. Contains:
- `gp->inBattle` — whether currently in battle
- `gp->mDeltaGame` / `gp->mDeltaRender` — frame timing
- Button input, system state, and more

## Working with the Codebase

### Agent Tools (under `tools/`)

TTYD provides Python tools for agents to triage work and audit correctness. All
tools assume the working directory is the repo root and read
`build/G8MJ01/report.json` (regen with `ninja build/G8MJ01/report.json`).

| Tool | Purpose | Key flags |
|------|---------|-----------|
| `tools/stub_queue.py` | Rank undecompiled functions by quick-win likelihood | `--max 50`, `--max-size 200`, `--aligned-only`, `--game-only` |
| `tools/untouched_units.py` | List 0% matched units, classify A (missing) vs B (present) | `--flavor A`, `--game-only`, `--max 30` |
| `tools/function_order_check.py` | Verify function order for `-inline deferred` (DOL) or NORMAL (REL) | `<unit>`, `--all-mismatch`, `--rel` |
| `tools/scope_audit.py` | Audit symbol scope mismatches between source and symbols.txt | `<unit>`, `--extern-refs`, `--missing` |
| `tools/symbol_neighbors.py` | Find symbols near an address or name | `<query>`, `-n 10`, `--include-rels` |
| `tools/unit_info.py` | Detailed per-unit status with all function fuzzy %s | `<unit>`, `--show-asm`, `--json` |
| `tools/progress.py` | Project progress by category, top matching, largest untouched | `--category`, `--top 10`, `--json` |
| `tools/find_function.py` | Find which unit contains a function symbol | `<symbol>`, `--fuzzy`, `--rel <area>` |
| `tools/data_section_warn.py` | Identify files that can't pass SHA1 due to `.strtab` ordering | `--matching-only`, `<unit>` |
| `tools/decompctx.py` | Generate decomp.me ctx.c from a source file (existing) | `<file.c>`, `-I <include>` |

**Quick triage recipe** (most useful as a single command):
```powershell
python tools/untouched_units.py --game-only --max 30
```

This shows the 30 smallest untouched game-code units, classified by flavor.
Pick a flavor-A unit (asm symbols absent from .c) for the highest-yield work.

**Verify a file is ready to flip Matching**:
```powershell
python tools/data_section_warn.py "MarioSt/driver/swdrv"     # confirm no data sections
python tools/function_order_check.py "MarioSt/driver/swdrv"  # confirm function order
python tools/scope_audit.py "MarioSt/driver/swdrv"          # confirm scope matches
```

### Key tasks an agent may help with

1. **Matching functions** — Given a NonMatching function, analyze objdiff output and modify source to produce byte-identical assembly.

2. **Initial decompilation** — For functions that exist in assembly but have no source yet, recover the C code from the disassembly.

3. **Naming variables** — Many variables retain placeholder names (`a1`, `unk*`, `field_0x*`). Study usage patterns to propose descriptive names.

4. **Recovering structs** — Identify struct layouts from memory access patterns in assembly. Cross-reference with header definitions.

5. **Understanding game mechanics** — Read decompiled functions and add comments explaining what game mechanics are being implemented.

6. **Data recovery** — Recover data definitions, rodata ownership, string tables, and float constants from assembly data sections.

### Important things to keep in mind

- **Never guess assembly**. All decompiled code must be verified against the original binary via objdiff.
- **Always use tools for arithmetic**. When computing sizes, offsets, instruction counts, or any numeric conversions, use Python or a terminal command instead of mental math. LLMs are unreliable at arithmetic.
- **Order matters**. Functions and global variables must appear in the same order as in the original binary. With `-inline deferred` (used for most game code), emission order is **reversed** from source order.
- **Include order matters**. It affects BSS layout and static initialization order.
- **The compiler is MWCC, not GCC/Clang**. It has unique optimization behavior, instruction selection, and register allocation. Code that looks equivalent in C may compile differently. The only ground truth is objdiff comparison.
- **This is C, not C++**. No classes, no templates, no vtables, no name mangling. Function names in `symbols.txt` are plain C names.
- **REL modules use different flags**. RELs compile with `-O4,s` (size optimization) and `-sdata 0 -sdata2 0` (no small data). This affects code generation.

### Function Order with `-inline deferred`

The `-inline deferred` flag causes the compiler to **reverse** function emission order. This means:

- **Source order**: `funcA`, `funcB`, `funcC`, `funcD`
- **Binary order**: `funcD`, `funcC`, `funcB`, `funcA`

To determine the correct source order:
1. Check the original object's symbol order using `dtk elf info <obj>.o`
2. Reverse that order for the source file
3. The first function in the binary should be LAST in the source, and vice versa

**Example**: If the binary has functions in order `dispGetCurWork`, `dispCalcZ`, `dispDraw`, `dispSort`, `_sort`, `dispEntry`, `dispReInit`, `dispInit`, then the source should have them in reverse: `dispInit`, `dispReInit`, `dispEntry`, `_sort`, `dispSort`, `dispDraw`, `dispCalcZ`, `dispGetCurWork`.

### Symbol Scope Issues

The `symbols.txt` file marks symbols with `scope:global` or `scope:local`. If a symbol is `scope:local` in the binary, it must be declared `static` in the source. If the source declares it as non-static but the binary has it as local, the linker will fail with undefined symbol errors when the file is marked as Matching.

**Example**: `seq_data` in `seqdef.c` is marked `scope:local` in `symbols.txt`, so it must be `static` in the source. But `seqdrv.c` references it as `extern` - this is a contradiction that needs to be resolved.

### 100% Fuzzy Match ≠ Byte-Identical

**Critical**: A file showing "100% fuzzy match" in objdiff does NOT guarantee it will pass the SHA1 hash check. The fuzzy match percentage only measures instruction-level matching in the `.text` section. A file can have 100% instruction match but still fail the hash check due to:

- **Data section differences**: `.data`, `.rodata`, `.sdata`, `.sdata2` sections may have different content or missing data (e.g., large data arrays not defined in source)
- **BSS section differences**: Variable order, alignment, or padding in `.bss`/`.sbss` sections
- **Symbol scope mismatches**: Source defines a symbol as `global` but binary has it as `local` (static), or vice versa
- **Function order issues**: With `-inline deferred`, source order must be reversed from binary order
- **Compiler-generated padding**: The compiler adds alignment padding that's difficult to control from source
- **Missing data definitions**: Data-heavy files (item tables, NPC data, mail data) need ALL data defined in source to match

**Strategy for data-heavy files**: Don't try to flip data-heavy files (like `data/item_data.c`, `data/mail_data.c`, `data/npc_data.c`) to Matching unless the source contains ALL the data from the original object. The objdiff report's `complete_code_percent` and `complete_data_percent` are more reliable indicators than `fuzzy_match_percent`.

**Best candidates for Matching**: Files with:
- No data sections (code-only files like `driver/swdrv.c`)
- Small or no `.bss`/`.sbss` sections
- Correct function order (reversed for `-inline deferred`)
- Correct symbol scopes (matching `symbols.txt`)

### Matching workflow for agents

1. **One change at a time**. Make a single source-level change, rebuild with `ninja`, and check the diff immediately. Even small changes can have non-local effects on register allocation and instruction scheduling.

2. **Focus on structural mismatches first**. In diff output, `|` (opcode replacement), `<` (deleted), and `>` (inserted) are the real issues. `~` (operand-only) mismatches are usually just register renumbering or stack offset differences that resolve once structural issues are fixed.

3. **Read the full diff before acting**. Identify all problem clusters before making changes, then work on the largest cluster first.

4. **Focus on one part of the function at a time**. Identify what source lines correspond to non-matching assembly regions.

5. **Use temporary marker calls to map source to asm when anchors are lacking**. Add `extern void marker__();` and call it at a candidate point. The call shows up clearly in diff output. **Always remove markers after mapping** — extra calls change register allocation.

### Forcing MWCC instruction selection

When a function is close to matching but has instruction-selection mismatches, these source-level rewrites sometimes help:

| Source pattern | Try instead |
|---|---|
| `x &= 0xfffffbff;` | `x &= ~0x400;` (may change rlwinm vs li+and) |
| `*(int*)p != 0` | `*(void**)p != NULL` (signed vs unsigned compare) |
| `if (v <= K) return v; return K;` | `if (v > K) v = K; return v;` (clamp pattern) |

`#pragma peephole off` + `#pragma scheduling off` around a function can fix peephole-related mismatches (e.g., `extsb.` fusion differences).

For stubborn instruction-selection mismatches, inline `asm {}` blocks with `register` variables are the last resort. Declaration order of `register` variables controls which physical register MWCC assigns.

### When to NOT chase matching

- The diff is only scheduling reorder (mr/li swap, prologue save order). There may be nothing reasonable from source.
- Function is below ~50% matched. Solve the bulk of the body first, then come back for polish.
- The surrounding code is clearly wrong (bad function boundaries, missing functions). Fix structure first.

## REL-Specific Guidance

REL modules have different compilation characteristics:

- **Compiler**: Same `GC/2.6` but with `-O4,s` (optimize for size, not speed).
- **No small data**: `-sdata 0 -sdata2 0` means no `.sdata`/`.sdata2` sections. All data access goes through full 32-bit addresses.
- **Self-contained**: Each REL is a standalone module loaded at runtime. Symbols are resolved against the DOL's exports.
- **Per-module config**: Each REL has its own `symbols.txt` and `splits.txt` under `config/G8MJ01/<area>/`.
- **Source location**: Active REL work lives under `rels/<area>/src/` with headers in `rels/<area>/include/`.

When working on REL code, be aware that the different optimization level (`-O4,s` vs `-O4,p`) produces different code generation patterns than the DOL code.

## Commit Standard

Commit when both are true:
- The repo is materially better: codegen, data, linkage, structure, or tooling improved.
- The result is plausible original source or materially improves the ability to recover it.

Do not commit changes that break the build. Always verify `ninja` succeeds after changes.

## Session Progress (2026-05-31)

### Files Fixed This Session

**`mario/mario_pouch.c`** (2 functions fixed):
- `pouchGetStarPoint`: Changed return type from `u16` to `s16` in both `src/mario/mario_pouch.c` and `include/mario/mario_pouch.h`. The original uses `lha` (signed halfword load) while the compiled was using `lha` + `clrlwi` (zero-extend). Fixed by matching the struct field type.
- `pouchGetYoshiName`: Uncommented the `msgSearch("name_party3")` call (with cast to `char*`) instead of returning `NULL`. The original calls `msgSearch` when the name is empty.

**`manager/dvdmgr.c`** (attempted `DVDMgrRead` fix - partial):
- Added local variable `DVDEntry* entry2 = entry;` to force the compiler to use r31 as a callee-saved register. This matched the original's `mr r31, r3` pattern.
- However, the loop structure still doesn't match. The compiler generates `beq` to loop while the original uses `bne` to exit. This is a fundamental compiler optimization that can't be easily changed from source.
- Reverted to the simpler while loop since the hash still matches (linker handles the differences).

**`event/evt_env.c`** (in progress - 41.94% match, 248B vs 224B original):
- Decompiled `evt_env_blur_on` (single function, original 0xE0 = 224 bytes).
- Started at 0% / 280B (over by 56B / 14 instr). Now at 41.94% / 248B (over by 24B / 6 instr).
- **Major fix**: userdata[0]/[1] lifetime storage was backwards. Original stores userdata[0] = lifetime HIGH (big-endian: high at low address) and userdata[1] = lifetime LOW, matching `evt_wait_msec` pattern at `evtmgr_cmd.c:275`.
- **Verified struct layout**: `event->userdata[0]` at 0x78, `[1]` at 0x7C, `[2]` at 0x80. Big-endian: `lwz r3, 0x0(r30)` reads HIGH 32 bits of `event->lifetime`.
- **Optimization: hoist lifetime load before isFirstCall check** (load `lifetimeHi`/`lifetimeLo` as local variables) to enable 5 callee-saved registers and shrink code from 280B to 248B.
- Remaining 6 extra instructions are compiler quirks:
  1. `li r6, 0; mullw r6, r6, r7; add r0, r0, r6` (compiler does full 64-bit mul instead of `mulhwu + srwi`)
  2. `xoris r5, r3, 0x8000; xoris r3, r0, 0x8000` (sign-flip comparison pattern instead of `srawi + subfe chain`)
  3. `b` extra branch (inverted `beq` vs original's `bne` semantics)
- **Key insight**: `(u32) * (u32) >> 32` is undefined in C; compiler may optimize to 0 (we discovered this with a 220B fakematch). Use `(u64)(u32) * (u64)(u32) >> 32` for the high 32 bits.
- **32-bit fakematch**: Using `(u32)(((u32)(n >> 2) * 0x10624DD3u) >> 16)` gave 224B size match but with WRONG math (compiler uses 32-bit mul + shift 16, not mulhwu + shift 6).

### Key Findings

1. **Object file vs final binary differences**: Some functions show <100% match in objdiff reports but still produce the correct final binary. This is because:
   - The original object file has relocations (e.g., `R_PPC_REL24`) with placeholder bytes
   - The compiled object file may have pre-computed displacements
   - The linker patches both to the same final value
   - Example: `OSGetTime` shows 83.3% match but final DOL is correct
   - Example: `LCQueueWait` shows 80% match but final DOL is correct

2. **Type mismatches cause instruction differences**:
   - `s16` vs `u16` return types affect whether `lha` (sign-extend) or `lhz` (zero-extend) is used
   - The struct field type should match the return type for getters
   - `u32` vs `u64` affects whether `mulhwu` (32-bit high mul) or full 64-bit mul is used

3. **Commented-out code may need to be uncommented**:
   - `pouchGetYoshiName` had `// return msgSearch("name_party3");` which needed to be uncommented to match

4. **Local variables can help with register allocation**:
   - Adding `DVDEntry* entry2 = entry;` and using `entry2` throughout the function can force the compiler to use r31 as a callee-saved register, matching the original's `mr r31, r3` pattern.
   - Hoisting `lifetimeHi`/`lifetimeLo` to local variables enables the compiler to keep them in callee-saved registers (shrinks code 280B → 248B).

5. **Loop structure is hard to match**:
   - The original uses `bne` to exit a loop while the compiler prefers `beq` to loop back
   - `do-while` loops and `goto` patterns don't reliably force the original's loop structure
   - To match exactly, inline assembly or pragma directives are needed

6. **64-bit operations on u64 have specific patterns**:
   - **Multiplication shortcut**: `mulhwu r0, X, Y; srwi r6, r0, 6` (2 instr) = `(X * Y) >> 38` (top 26 bits of 64-bit product)
   - **Compiler's 5-instruction form**: `mulhwu r0, X, Y; li r5, 0; mullw r5, r5, Y; add r0, r0, r5; srwi r6, r0, 6` (5 instr)
   - **Signed comparison pattern** (original): `srawi r5, rX, 31; subfc r0, rX, r4; subfe r0, r5, r3; subfe r0, r29, r29; neg. r0, r0; bne ...`
   - **Compiler's xoris form**: `srawi r0, rX, 31; xoris r5, r3, 0x8000; xoris r3, r0, 0x8000; subfc r0, rX, r4; subfe r3, r3, r5; subfe r3, r5, r5; neg. r3, r3; beq ...`

7. **MWCC `register` keyword doesn't force specific register assignment**:
   - GCC-style `register int x asm("r26")` doesn't work
   - Declaration order MAY influence physical register choice, but doesn't guarantee 6 callee-saved registers

8. **Compiler quirk: `li r3, -1; and r0, rx, r3`** is a "narrow to 32 bits" pattern emitted for `(s32)x` cast when x was loaded as wider type. The original uses this for `event->userdata[0/1] = (s32)lifetimeHi/Lo` casts.

**`event/evt_ext.c`** (FLIPPED TO MATCHING - 184/184B, 2/2 functions, 100%):
- Decompiled `evt_ext_reset` (36B) and `evt_ext_entry` (148B). Both functions at 100% instruction match.
- **`USER_FUNC(evt_ext_reset)`**: calls `extReset()` (external), returns `EVT_RETURN_DONE` (2).
- **`USER_FUNC(evt_ext_entry)`**: takes 5 args via `evtGetValue(event, args[i])`, calls `extEntry(v0,v1,v2,v3,v4)`, returns `EVT_RETURN_DONE`. Uses 5 callee-saved registers (r27-r31), `r31` holds `args` pointer.
- **CRITICAL FIX**: Function order was reversed in source. With `-inline deferred`, the **first function in source becomes LAST in binary, and vice versa**. I had `reset` first, `entry` second, but original binary has `reset` first, `entry` second — meaning source must be `entry` first, `reset` last. The objdiff showed 100% fuzzy match but the linker failed because the .text was in wrong order.
- `.note.split` is not required for our compiled .o (the linker doesn't need it because the .o position in the link line is fixed). The original .o has it from dtk extraction; our compiled .o doesn't, and that's fine.
- `MarioSt.dol` SHA1: `cf559d97fef1b3efb8788126250aee88f0491410` (matches).

**`effect/eff_ripple.c`** (4/4 functions 100% matched at 204/204B, but NOT flipped):
- All 4 functions (`effRippleSetPosition` 124B, `effRippleEntry` 36B, `effRippleSetCamId` 20B, `effRippleSetRxRz` 24B) match perfectly. But the file has data sections (`.sdata2` 24B) with different padding than original, so SHA1 would fail. Left as NonMatching in `configure.py:465`.

**`event/evt_paper.c`** (FLIPPED TO MATCHING - 196/196B, 2/2 functions, 100%):
- Decompiled `evt_paper_delete` (72B) and `evt_paper_entry` (124B). Both functions at 100% instruction match.
- **`USER_FUNC(evt_paper_entry)`**: takes string arg via `evtGetValue`, calls `animGroupBaseAsync(name, inBattle, NULL)`. If returns 0, return 0. Otherwise call `animPaperPoseEntry(name, inBattle)` and return EVT_RETURN_DONE.
- **`USER_FUNC(evt_paper_delete)`**: takes string arg, calls `animPaperPoseRelease(animPaperPoseGetId(name, inBattle))`, returns EVT_RETURN_DONE.
- **CRITICAL PATTERN**: `neg; or; srwi 31` is a **non-zero check** (not sign check), used to convert s32 to 0/1. The C must be `((u32)(-(s32)x) | (u32)x) >> 31` with operands in **specific order** to match: the negated value on the LEFT so the OR reads `or r0, r0, r4` (not `or r0, r4, r0`).
- **Function order fix**: source must have `entry` first, `delete` last (because of `-inline deferred`). objdiff showed 100% match but linker SHA1 failed until order was reversed.
- Uses `gp->inBattle` (at 0x14 in GlobalWork, NOT 0xF8).
- `MarioSt.dol` SHA1: `cf559d97fef1b3efb8788126250aee88f0491410` (matches).

**`event/evt_snd.c`** (3 functions, 93.33% best match, NOT flipped - 1-instruction encoding diff):
- Decompiled `N_evt_snd_bgm_unpause` (60B, 93.33% match), `seq_gameOverExit` (40B, 100%), `seq_gameOverInit` (96B, 100%).
- `N_evt_snd_bgm_unpause`: gets `psbgm.streamId`, checks `(u32)(streamId + 0x10000) != 0xFFFFu` (compiler's "is -1?" check), then clears bottom 4 bits of u16 at `*(u16*)((char*)(*(u32*)(sound+0x100)) + streamId*0x138)`.
- **`addis rX, rY, 0x1; cmplwi rX, 0xFFFF` pattern** detects `rY == 0xFFFFFFFF` (i.e., -1). C must be `if ((u32)(streamId + 0x10000) != 0xFFFFu)` not `if (streamId != -1)`.
- **`lhzx` (load halfword indexed)** pattern: `(u16*)(base + byteOffset)`. The byte offset is `streamId * 0x138`. The C must keep base and offset as separate variables for the compiler to use lhzx (not lhz with offset).
- **Sound struct is at `sound+0x100`** (loaded as a pointer). The struct stride is 0x138 bytes. Field at offset 0 of struct is the u16 flags.
- **Bit-clear `& 0xFFF0` vs `& 0xFFFE`**: original clears bottom 4 bits, my compiler generates same semantic but different encoding. The bytes `54 00 04 7E` (orig, mask 4-31) vs `54 00 04 36` (mine, mask 4-13). The compiler chose different mask for u16 because it knows the top 16 bits are already 0.
- `seq_gameOverExit`/`seq_gameOverInit`: void functions that call various psnd/clearFlag/marioSt functions. Both 100% match.
- **Function order**: source must have `seq_gameOverInit` first, `seq_gameOverExit` second, `N_evt_snd_bgm_unpause` last (reversed in binary).
- **External funcs** declared in evt_snd.c: `L_camDispOn`, `L_psndBGM_stop`, `N_pouchUnEquipBadgeID`, `psndSetReverb`.
- Can't flip to Matching because the 1-instruction `rlwinm` encoding diff causes SHA1 mismatch.

**`effect/eff_confetti.c`** (88.9% fuzzy, NOT flipped - 2 unfixable diffs):
- Single function `effConfettiEntry` (36B) loads 1.0f into f4 and calls `effConfettiN64Entry`.
- **Unfixable diff #1: register choice (f1 vs f4)**: To get 1.0f in f4, the prototype must have 3+ prior float args. With `(f32, f32, f32, f32)` prototype and `effConfettiN64Entry(0.0f, 0.0f, 0.0f, 1.0f)`, my compiler emits 12 instructions (48B) — too big. The original emits 9 instructions (36B). The original compiler OMITTED the explicit 0.0f loads for f1, f2, f3 — possibly because the function doesn't actually use them, or the source uses a non-standard calling pattern.
- **Unfixable diff #2: .sdata2 size/order (4B vs 8B)**: Original has `float_1_8041c730` (4B) + `gap_09_8041C734_sdata2` (4B of 0). My compiler emits only `float_1` (4B total) — the gap is uninitializable from C source. The compiler optimizes out unused `static const f32 float_0 = 0.0f;` declarations. Even `volatile const` is optimized out. Without interprocedural optimization (knowing the function doesn't read the gap), the gap can't be forced.
- **Tried prototypes**: `(f32)` 88.9%, `(f32, f32, f32, f32)` 66.7%, `(s32, s32, f32, f32, f32, f32)` 57.1%, K&R `()` 88.9%.
- **Tried data tricks**: explicit `static const f32 float_0 = 0.0f`, `volatile const`, array `[1] = {0}`, using both constants in a 6-arg call.
- **Conclusion**: Cannot reach 100% without knowing the original source's exact pattern. Left as NonMatching.

### Current Build Status
- **Total units**: 633, **Matching**: 140 (22.1%) — up from 138
- **Game Code Matching**: 13/416 (up from 11)
- **Total functions**: 7063, **Matched**: 1782 (25.2%) — up from 1772
- **Total units (all)**: 107/1239 matched, 7.52% linked
- **DOL hash**: `cf559d97fef1b3efb8788126250aee88f0491410` (matches expected)

