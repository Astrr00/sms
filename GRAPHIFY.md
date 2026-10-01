# Graphify

Cursor can query a local knowledge graph in `graphify-out/`.
This graph was built with graphifyy 0.9.73 from the repository root, not a `src/` subset.
Extraction is AST-only.
No LLM API key is used.

Install once (Python 3.10+):

```bash
uv tool install graphifyy
```

The console command is `graphify`.
`graphify cursor install` writes `.cursor/rules/graphify.mdc` (`alwaysApply: true`).

Headless rebuild (no model):

```bash
export PYTHONHASHSEED=0
graphify extract . --code-only
graphify cluster-only .
```

`graphify extract` writes `graph.json`.
`graphify cluster-only` writes `GRAPH_REPORT.md`, `graph.html`, and the label sidecars.
Community names are the highest-degree hub in each community.

`--code-only` still indexes Markdown here, because `.md` is classified as code and `extract_markdown` is a local heading/link walk, not an LLM pass.
YAML, plain text, and the PNG image stay skipped.
Those formats need a model, and this environment has no API key.

Also indexed beyond stock extensions: `.tpp`, `.cp`, `.pch`, and extensionless MSL C++ headers (`algorithm`, `iterator`, `memory`, `new`, `cstddef`, `cstdint`, `msl_utility`).
A non-Pascal `.inc` is parsed as C.
`src/TRK_MINNOW_DOLPHIN/__exception.s` is not indexed (no PowerPC assembly grammar).

Metrowerks dialect that tree-sitter rejects (`extern "C"`, `__attribute__`, call-site `AT_ADDRESS`, `#define AT_ADDRESS(addr) : (addr)`, `#error` in inactive branches, and `asm` / mnemonic lines in files that already contain `asm`, `__MWERKS__`, or `nofralloc`) was blanked to spaces before parse, keeping line numbers.
`.h` and `.c` are parsed with both the C and C++ grammars and the richer tree is kept.
That normalizer lives only in the graphify install used to build this graph.
A stock `graphify update .` on another machine will not repeat it, and can drop the recovered nodes.

Refresh after ordinary code changes, once that same extractor is available:

```bash
export PYTHONHASHSEED=0
graphify update .
```

Query:

```bash
graphify query "what calls TMario"
graphify path "TLiveActor" "TFireWanwan"
graphify explain "TSpineEnemy"
```

Serve the graph over MCP (stdio):

```bash
graphify-mcp graphify-out/graph.json
```

`python -m graphify.serve graphify-out/graph.json` is the same server.
Run it with the interpreter that has `graphifyy` installed (`uv tool` does not put that module on the system `python3`).

`graphify-out/cache/` is gitignored and rebuilt on the next run.
Do not commit `graphify-out/.graphify_root` (absolute machine path) or dated `graphify-out/YYYY-MM-DD/` backups.
Commit `graph.json`, `GRAPH_REPORT.md`, `graph.html`, and the label sidecars.

`graph.html` is a community aggregation (the full graph is above graphify's 5000-node HTML cap).

## Coverage

From `graphify-out/GRAPH_REPORT.md`, compared with the previous code-only graph (42,273 nodes, 73,663 edges, 1,260 communities, 1,494 files, 158 syntax warnings):

- 44,587 nodes (+2,314)
- 77,359 edges (+3,696)
- 1,389 communities (+129)
- 84% EXTRACTED, 16% INFERRED, 0 LLM tokens
- 1,515 code files scanned, 1,514 distinct `source_file` values

Newly present versus that pass (20 files, nothing removed):

- Markdown: `README.md`, `GOAL.md`, `PROGRESS.md`, `GRAPHIFY.md`, `CLAUDE.md`, `GEMINI.md`, `docs/AGENT_MATCHING_TIPS.md`, `docs/PROGRAM_STRUCTURE_REVVING.md`
- `AGENTS.md` is byte-identical to `CLAUDE.md` and `GEMINI.md`, so the merge kept one copy
- `.tpp`: `JDRNameRefPtrList.tpp`, `JDRViewObjPtrList.tpp`
- `.cp`: `ExceptionPPC.cp`, `NMWException.cp`
- `include/SMS.pch`
- extensionless MSL headers under `include/PowerPC_EABI_Support/Msl/MSL_C++/MSL_Common/`

The same files also gained 2,041 nodes across 95 translation units once the C++ grammar and the Metrowerks normalizer ran.
The largest recoveries are `include/dolphin/gx/GXEnum.h` (+734), `include/TRK_MINNOW_DOLPHIN/ppc/Generic/targimpl.h` (+119), and `src/dolphin/gx/__gx.h` (+101).
Graphify still does not emit nodes for function prototypes, typedefs, or namespace-scope globals, so a clean prototype-only header stays a single file node.

## Remaining parse failures

143 files still contain a recovered tree-sitter error.
Graphify warns only when the file yields nothing beyond its file node, or the error spans multiple lines.
That warning list is 36 files (down from 158).

No symbols (6):

- `include/Player/MarioAnimeFiles.inc` (initializer fragment, not a translation unit)
- `include/PowerPC_EABI_Support/Msl/MSL_C++/MSL_Common/cstdint`
- `include/PowerPC_EABI_Support/Runtime/__mem.h`
- `include/dolphin/__ppc_eabi_init.h`
- `include/dolphin/gd/GDVert.h`
- `include/dolphin/gx/GXVert.h`

Partial, symbols kept, multiline error remains (30), mostly inline `asm` under `__MWERKS__`:

- `include/JSystem/J3D/J3DGraphBase/J3DTransform.hpp`
- `include/JSystem/JGeometry/JGMatrix34.hpp`
- `include/dolphin/os.h`
- `src/JSystem/J3D/J3DGraphAnimator/J3DAnimation.cpp`
- `src/JSystem/J3D/J3DGraphAnimator/J3DModel.cpp`
- `src/JSystem/J3D/J3DGraphBase/J3DTransform.cpp`
- `src/M3DUtil/SampleCtrlModel.cpp`
- `src/MarioUtil/MathUtil.cpp`
- `src/PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c`
- `src/PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_atan.c`
- `src/PowerPC_EABI_Support/Runtime/runtime.c`
- `src/TRK_MINNOW_DOLPHIN/debugger/embedded/MetroTRK/Os/dolphin/dolphin_trk.c`
- `src/TRK_MINNOW_DOLPHIN/debugger/embedded/MetroTRK/Processor/ppc/Generic/flush_cache.c`
- `src/TRK_MINNOW_DOLPHIN/debugger/embedded/MetroTRK/Processor/ppc/Generic/mpc_7xx_603e.c`
- `src/TRK_MINNOW_DOLPHIN/debugger/embedded/MetroTRK/Processor/ppc/Generic/targimpl.c`
- `src/dolphin/ai/ai.c`
- `src/dolphin/gx/GXTransform.c`
- `src/dolphin/mtx/mtx.c`
- `src/dolphin/mtx/mtxvec.c`
- `src/dolphin/mtx/vec.c`
- `src/dolphin/os/OS.c`
- `src/dolphin/os/OSAlarm.c`
- `src/dolphin/os/OSCache.c`
- `src/dolphin/os/OSContext.c`
- `src/dolphin/os/OSMemory.c`
- `src/dolphin/os/OSReset.c`
- `src/dolphin/os/OSSync.c`
- `src/dolphin/os/__ppc_eabi_init.cpp`
- `src/dolphin/os/__start.c`
- `src/dolphin/thp/THPDec.c`

Not in the graph at all: `__exception.s`, four `.yml` workflows/configs, and four `symbols.txt` / `splits.txt` files.
Unclassified leftovers are dotfiles, `LICENSE`, two `build.sha1` files, and `.gitkeep` placeholders.
