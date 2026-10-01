# Graphify

Cursor can query a local code knowledge graph in `graphify-out/`.
The graph is AST-only (`graphifyy` / tree-sitter).
Docs, papers, and images are not indexed, and no LLM API key is required.

Install once (Python 3.10+):

```bash
uv tool install graphifyy
```

The console command is `graphify` (this graph was built with graphifyy 0.9.73).
`graphify cursor install` writes `.cursor/rules/graphify.mdc` (`alwaysApply: true`).

This repo was indexed from the root with:

```bash
graphify extract . --code-only
graphify cluster-only .
```

`graphify extract` writes `graph.json`.
`graphify cluster-only` writes `GRAPH_REPORT.md` and `graph.html` without calling a model.
Community names are the highest-degree hub in each community.

Refresh after code changes:

```bash
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
Commit `graph.json`, `GRAPH_REPORT.md`, and `graph.html` so the map stays in git.

`graph.html` is a community aggregation (the full graph is above graphify's 5000-node HTML cap).
The first extract reported 158 files with tree-sitter syntax errors, mostly headers, so those symbols may be partial.
See `graphify-out/GRAPH_REPORT.md` for node and edge counts.
