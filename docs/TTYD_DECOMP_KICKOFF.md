# TTYD-Decomp — Team Kickoff Prompt

Paste this whole document as the kickoff prompt for a multi-agent TTYD decomp
session. It assumes the working directory is the repo root and the repo is
on branch `main` tracking `origin/main`.

---

You're working on the **Paper Mario: The Thousand-Year Door (TTYD)**
decompilation project at the repo root.

- **Target**: JP v1.0 (`G8MJ01`, GameCube). Config in `config/G8MJ01/`.
- **Compiler**: Metrowerks CodeWarrior `GC/1.3.2` (version string `2.4.2.1`)
  for game code, RELs, and MusyX. SDK uses `GC/1.2.5n` (`2.3.0.1`).
- **ISO**: Japanese release. US `G8ME01` does **not** match the config hashes.
- **Read first**: `AGENTS.md` (project runbook + matching playbook + compiler
  behavior notes). Required reading — it documents the techniques already
  discovered and will save you from reinventing them.

## Single-agent workflow (CURRENT — no parallel worktrees)

This project currently runs as a **single-agent session** (no worktree fan-out
yet). All edits go straight to `main` in the working tree; rebuild with `ninja`
to see objdiff deltas. If/when parallel agents are introduced, mirror the
worktree setup from SFA's kickoff — symlink `orig/G8MJ01/` and run
`python configure.py` per worktree.

```powershell
# 1. Configure (run once, or after any configure.py / config.yml change).
python configure.py

# 2. Build & diff. ninja auto-regenerates objects on source change.
ninja

# 3. Regenerate the progress report if you need fresh numbers
#    (ninja does NOT auto-regen report.json).
Remove-Item build\G8MJ01\report.json -ErrorAction SilentlyContinue
ninja build\G8MJ01\report.json
```

If `ninja` says "no work to do", that's fine — objdiff (if open) has already
recompiled the latest code from your edits.

## Tooling

### Triage tools (run these FIRST when picking work)

**PRIMARY — find genuinely-untouched work from `report.json`.** A
`tools/untouched_units.py` helper script automates this:

```powershell
python tools/untouched_units.py --game-only --max 30
```

This shows the 30 smallest untouched game-code units, classified by flavor
(A: missing-from-src / B: present-but-unmatched). Pick a flavor-A unit for
the highest-yield work.

Pick fresh 0% units to hunt. **Big families of untouched work**:
- `MarioSt/MSL_C.PPCEABI.bare.H.a/*` — embedded MSL C math library
  (acos/asin/atan2/fmod/hypot/log/pow/rem_pio2/sqrt/cos/sin/tan/copysign).
  Small, self-contained, perfect for clean-C wins.
- `aji/`, `bom/`, `dou/`, `eki/`, `gon/`, `gor/`, `gra/`, `hei/`, `hom/`,
  `jin/`, `jon/`, `kpa/`, `las/`, `moo/`, `pik/`, `qiz/`, `tik/`, `tou/`,
  `tou2/`, `usu/`, `win/`, `yuu/` — REL module units. 1-function small
  helpers are common.
- `src/data/*` — data-heavy TUs (item/mail/NPC tables). These will NOT match
  (see blockers below) but are worth decompiling for documentation value.
- `src/unit/*` — enemy/unit behavior definitions. Many small `unit_kuriboo`,
  `unit_patakuri`, `unit_honenoko` family helpers still untouched.

**Two flavors of 0%-matched unit — only flavor A is high-yield clean-C work:**
- **(A) MISSING-from-src (drift):** the asm has symbols that are NOT defined in
  the `.c`. These are the **add-new-function** wins — add the symbol as a
  new correctly-named function; they routinely hit 90–100% from clean C.
  *This is the productive pool.*
- **(B) present-but-unmatched:** the function body is ALREADY in the `.c`
  (m2c/Ghidra-imported) but compiles to non-matching code. These are
  partial-improvement only and the residuals are usually
  register-allocation/scheduling — i.e. asm-territory we skip. Low yield;
  don't mistake a flavor-B unit for fresh work.

Distinguish them per unit: open the `.c`. If the 0% functions already have
full bodies, it's flavor B (skip); if the asm symbols are absent from the
`.c`, it's flavor A (go). For REL units the `.c` may be entirely absent
(`rels/<area>/src/<unit>.c` doesn't exist yet) — that's flavor A by
definition.

### Diff/inspection

- **objdiff-cli** (agent-friendly, non-interactive):
  `build\tools\objdiff-cli diff -p build\G8MJ01 <unit_name>`
- **List all units and status**:
  `build\tools\objdiff-cli report -p build\G8MJ01`
- **Detailed unit status** (per-function fuzzy %s, file paths, sizes):
  `python tools\unit_info.py <unit_name>` (e.g. `python tools\unit_info.py "MarioSt/driver/swdrv"`)
- **Find which unit contains a function**:
  `python tools\find_function.py <symbol> --fuzzy`
- **Find symbols near an address/name**:
  `python tools\symbol_neighbors.py 0x800599A0 -n 10`
- **Verify a unit is ready to flip Matching** (data sections, function order, scope):
  ```powershell
  python tools\data_section_warn.py "MarioSt/driver/swdrv"
  python tools\function_order_check.py "MarioSt/driver/swdrv"
  python tools\scope_audit.py "MarioSt/driver/swdrv"
  ```
- **Generate decomp.me scratch context** from a unit's asm:
  `python tools\decompctx.py <unit_path>` (e.g. `python tools\decompctx.py src\driver\swdrv.c`)
- **Per-TU asm output**: `build\G8MJ01\asm\<path>.s` — disassembly with all
  data sections (`.rodata`, `.data`, `.sdata2`) intact. Use this for
  recovering literal values (strings, floats, jump tables).
- **Compiled object**: `build\G8MJ01\src\<path>.o`
- **Target (original) object**: `build\G8MJ01\obj\<path>.o`
- **dtk** (decomp toolkit): `build\tools\dtk.exe` — for splitting the DOL/REL
  into per-TU objects and inspecting ELF info.
  - `dtk elf info <obj>.o` — shows symbol order in the binary (essential for
    verifying function order in source vs binary order).
- **m2c** (optional, external): asm-to-draft decompiler. Use only as
  orientation:
  `python /path/to/m2c/m2c.py -t ppc -f <function_name> --globals=none build\G8MJ01\asm\<path>.s`
  Treat its output as scaffolding for understanding control flow; never trust
  it over the asm or objdiff.

## Workflow

0. **Targeting reality (read once).** The project sits around ~13% fuzzy /
   18% of functions / 10% of *bytes* matched. The function%≫byte% gap means
   the unmatched code is dominated by **large FP/GX/matrix/render/physics
   functions** and **data-heavy TUs** — these are partial/untouched. The
   clean-C wins live in the **many SMALL functions** (init/update/free/
   dispatch/logic/getter/setter) inside fresh 0% units, especially in the
   `MSL_C.PPCEABI.bare.H.a` math library and the `unit_*` enemy helpers.
   So target small functions in untouched units for fuzzy%/function-count
   gains.
   **Nuance — "skip data-heavy" means skip GRINDING existing partials for
   100% match, NOT skip DECOMPILING them.** Writing a *missing*
   (flavor-A, absent-from-src) function as clean C that lands at 85-99% is
   a REAL win — it recovers plausible source and is a big 0→high fuzzy gain.
   Even FP-heavy missing functions land high from clean C. So DO add missing
   FP functions — take the natural % and leave a clean partial. Only *skip*:
   (a) grinding an existing partial for the last few %, and (b) the
   genuinely giant bodies (1k-40k bytes) where a single mismatch tanks the
   whole function.
1. **Triage**: run the report.json 0%-unit query above to get fresh untouched
   units; pick one no other agent owns.
2. **Pick a fresh 0% unit** (or a 0–25% unit) and claim it as sole owner.
   Bias to units with several small functions for throughput.
3. **For REL units, create the source file from scratch.** Many RELs have
   `rels/<area>/src/<unit>.c` that doesn't exist yet — start from a m2c draft
   or hand-write from the asm. Function order is NORMAL (not reversed) for
   RELs since they use `-O4,s` without `-inline deferred`.
4. **For DOL units, verify function order.** With `-inline deferred`, the
   binary's symbol order is **REVERSED** from source order. Use
   `dtk elf info <obj>.o` to confirm, then reverse the source.
5. **Read source + asm side-by-side**: `src\<path>.c` (or `rels\<area>\src\<path>.c`)
   and `build\G8MJ01\asm\<path>.s`.
6. **Diff one function**: `objdiff-cli diff` to see the instruction-level
   deltas. Note which source lines correspond to which asm regions.
7. **Apply ONE clean-C recipe at a time** from `AGENTS.md` (peephole pragma,
   `& ~constant` for `rlwinm`, `*(void**)p` for `cmplwi`, bitfield for
   `rlwimi`, source-form micro-tweaks, register coloring via local
   declaration order). Rebuild. Recheck. **NO inline asm** — leave the
   residual as a clean partial.
8. **Build hygiene before every commit**: `ninja` must succeed (exit 0).
   Warnings are NOT build breaks — only `error:` / `FAILED:` with non-zero
   exit is. If % drops, revert immediately. Clean up m2c phantom vars
   (`'extraout_f1'/'in_rN' not initialized`).
9. **Commit each meaningful gain** (>1% fuzzy, a function newly matched, or
   a structural fix that aligns the function set). See commit protocol below.
10. **Budgets**: 20 min hard limit per function. "Task complete" ≠ 100%
    match — "materially better with multiple gains" is enough. Move on rather
    than overfit one function. A clean 85–99% partial beats an asm-forced 100%.

## Commit & push protocol

```bash
git add -A
git commit -m "Recover <unit>: <function or change>"
git fetch origin main
git rebase origin/main
git push origin HEAD:main
# If push rejected: git fetch origin main && git rebase origin/main && retry.
```

- **NEVER** push `--force` to `main`.
- **NEVER** bypass git hooks (`--no-verify`).
- If you hit a rebase conflict in a file you don't own, abort with
  `git rebase --abort`, re-pull, and skip that file.

**Commit when both are true:** the repo is materially better (codegen, data,
linkage, structure, or tooling improved), AND the result is plausible
original source or materially improves the ability to recover it. Do not
commit changes that break the build — always verify `ninja` succeeds.

## Quick reference: matching recipes (TTYD-specific)

Try in order when a function is already 80-95%:

1. **`#pragma peephole off`** + **`#pragma scheduling off`** around a function
   can fix peephole-related mismatches (e.g., `extsb.` fusion differences).
2. **`& ~constant`** instead of `& 0xFF7F`-style literal → emits `rlwinm`
   instead of `li + and`.
3. **`*(void **)ptr != NULL`** instead of `*(int *)ptr != 0` → emits `cmplwi`
   (signed vs unsigned compare).
4. **`if (v > N) v = N; return v;`** for clamp patterns (may emit `blelr`).
5. **Match return type to struct field type** for getters — `s16` field
   wants `s16` return (uses `lha` sign-extend), not `u16` (`lhz` + `clrlwi`
   zero-extend).
6. **Tempo CSE**: lift repeated load to a local before multiple stores.
7. **Local declaration order controls register coloring** — reorder locals
   to fix a register-permutation residual. The declaration order of
   `register` variables controls which physical register MWCC assigns.
8. **Add a local alias to force callee-saved register usage**:
   `T* alias = arg;` then use `alias` throughout — can force `mr r31, r3`
   patterns.
9. **Uncomment code that's been commented out** — sometimes the original
   devs commented out code that was then re-enabled, and the commented-out
   version compiles to the same thing.
10. **NO inline `asm`** — leave residuals as clean partials and move on.

For function ORDER with `-inline deferred` (DOL only):

- The first function in the binary should be LAST in source, and vice versa.
- Verify with `dtk elf info <obj>.o` and compare to source order.

For SYMBOL SCOPE:

- If `symbols.txt` marks a symbol as `scope:local`, declare it `static` in
  source. Conversely, `scope:global` symbols must be non-static. A mismatch
  fails to link when the file is marked `Matching`.

## Known systemic blockers (don't burn cycles here)

- **Files with data sections** (`src/data/*`, `gxsub.c`, `seq_game.c`,
  `dispdrv.c`, most files with `.data`/`.sdata`/`.sdata2`): the MWCC
  compiler places static symbols before global symbols in the `.strtab`
  section, but the original binary has them in a different order. This is
  a fundamental compiler behavior that cannot be controlled from source.
  Result: even 100% instruction match won't pass the SHA1 hash check for
  these files. **Don't try to flip data-heavy files to Matching unless
  you can match the symbol order** (which currently requires non-source
  tooling). Just decompile them and leave them as `NonMatching`.

- **The `.note.split` section (132 bytes) added by dtk** doesn't exist in the
  original binary. dtk would need a flag to suppress this, or we accept the
  extra bytes as long as the rest matches.

- **Alignment padding (`gap_07`, `gap_09`, etc.)** in BSS must be explicitly
  added to source for files with `Matching` status. Look for these in the
  binary's `.bss`/`.sbss` and add `// padding` declarations.

- **Loop structure (do-while vs while)** cannot be reliably forced from C
  source. The original may use `bne` to exit a loop while MWCC prefers
  `beq` to loop back. Equivalent semantically, different branch patterns.
  Without inline asm, accept the residual.

- **Anonymous `.sdata2` constants** (`@23`, `@32`, `@72`): cap functions at
  ~98% unless we own the file's `.sdata2` split. Strings/floats pool into
  `.sdata2` in an order we don't fully control.

- **REL modules with `scope:local` symbols referenced from outside**:
  contradiction in the symbol map. Example: `seqdef.c::seq_data` is marked
  `scope:local` in `symbols.txt` but referenced as `extern` from `seqdrv.c`.
  Need a human judgment call to resolve (declare static and accept the
  link-time error, or change the symbol map).

- **Build artifact caching** (sccache/ccache around MWCC) would speed
  rebuilds significantly but is invasive — not yet wired up.

- **Japanese strings** (debug strings, NPC names, message data) need
  shift-JIS encoding via `build\tools\sjiswrap.exe` — already configured
  in `configure.py`. Don't try to write literal Japanese in source files
  without the wrapper, the compiler will reject it.

## Reference: game structure cheat-sheet

- **DOL library groups** (under `src/`): `battle/`, `battle/ac/`, `battle/sac/`,
  `data/`, `driver/`, `effect/`, `effect/n64/`, `event/`, `manager/`,
  `mario/`, `motion/`, `party/`, `sequence/`, `unit/`, `window/`, `sdk/`.
- **REL module families** (under `rels/`): 30+ modules, one per game area.
  Japanese abbreviations: `mri` (Mushroom Road), `gor` (Rogueport), `tik`
  (Petal Meadows), `hei` (Hooktail Castle), `nok` (Petalburg), `muj` (Twilight
  Town), `tou` (Twilight Trail), `usu` (Creepy Steeple), `rsh` (Excess
  Express), `aji` (Fahr Outpost), `bom` (Bomb-omb Battlefield), `dig`
  (Glitzville), `dou` (Pirate's Grotto), `eki` (Riverside Station), `gon`
  (Dragon's Lair), `gra` (Glitzville arena), `hom` (Rogueport Sewers), `jin`
  (Poshley Heights), `jon` (Poshley Sanctum), `kpa` (Bowser's Castle), `las`
  (Palace of Shadow), `moo` (The Moon), `pik` (Riverside Station), `qiz`
  (Quiz Show), `sys` (System), `tou2` (Twilight Trail pt 2), `win` (Pit of
  100 Trials), `yuu` (Keelhaul Key).
- **Global state**: `extern GlobalWork* gp;` — the main game state struct,
  accessed everywhere.
- **Event system**: bytecode-based, with `evt*` (commands) and `evtmgr*`
  (manager). 118+ opcodes in `evtmgr_cmd.c`. Scripts are arrays of
  `EventCommand` structs.
- **Driver pattern**: each subsystem has a `*drv.c` (`mapdrv`, `camdrv`,
  `npcdrv`, `effdrv`, `msgdrv`, `hitdrv`, `dispdrv`, `animdrv`, `mobjdrv`,
  `swdrv`, `itemdrv`, `windowdrv`, `fadedrv`, `shadowdrv`, `lightdrv`,
  `envdrv`).

## Reference: existing winning patterns from this project

- `src\manager\evtmgr.c`, `src\manager\evtmgr_cmd.c` — already Matching.
  Study for style/naming conventions.
- `src\driver\swdrv.c` — switched from NonMatching to Matching by fixing
  function order with `-inline deferred` reversal.
- `src\effect\eff_*.c` (butterfly, hit, stardust, breath_fire, small_star) —
  decompiled from scratch using m2c + manual cleanup, all Matching.
  Good template for fresh decomp of a small wrapper-style file.
- `src\effect\eff_ripple.c`, `src\effect\eff_confetti.c` — decompiled but
  NOT Matching (data section `.sdata2` ordering issue).
- `src\mario\mario_pouch.c` — `pouchGetStarPoint` matched by changing return
  type from `u16` to `s16` (matched struct field type). `pouchGetYoshiName`
  matched by uncommenting a `msgSearch("name_party3")` call.
