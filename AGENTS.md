# Rules for AI Agents

Shared rules for all AI agents working in this repository. For framework-specific
setup, see your framework's adapter file:

| Framework | Adapter File |
|-----------|-------------|
| Claude Code | [`CLAUDE.md`](CLAUDE.md) |

## Boundaries

### Always (proceed autonomously)

- Work on feature branches -- never commit directly to `main`
- Atomic commits: one logical change per commit
- Reference issue numbers in branches and commits when an issue exists
- Verify documentation claims against source code
- Run the build after code changes to verify compilation
- Use `--body-file` for multiline `gh` CLI content (not `--body`)

### Ask First (get human approval)

- Modifying instruction files (`AGENTS.md`, `CLAUDE.md`, skill files)
- Changing CI or build configuration (`CMakeLists.txt` at top level)
- Removing code with unclear usage or history

### Never (hard stops)

- Skip pre-commit hooks with `--no-verify`
- Commit secrets, credentials, or private keys
- Document from assumptions -- verify against source code
- Modify vendored third-party code (Togl, BWidget, RapidXML)
- Modify IVS copyright files in `GeoZui4D/main/widgets/` (Sv_Matrix4,
  Sv_Quaternion, Sv_Raster)

## Issue-First Policy

When a GitHub issue exists for the work, reference it in branch names and
commits (`Closes #<N>` or `Refs #<N>`). For trivial fixes (typos, minor doc
corrections), a dedicated issue is not required.

## Build & Test

```bash
# Full build from repo root
mkdir -p build && cd build && cmake .. && make -j$(nproc)

# Install locally
cd build && cmake --install . --prefix ../install

# Run the application
../install/bin/GeoZui4D [files...]
```

The top-level CMakeLists.txt builds both libgz4d and GeoZui4D. Sub-projects
can also be built independently (see README.md).

## Documentation Accuracy

- Never document from assumptions -- every claim about APIs, parameters, or
  behavior must be verified by reading the actual source code.
- Check [`docs/decisions/`](docs/decisions/) for existing Architecture Decision
  Records before proposing changes that might conflict.

## Post-Task Verification

Before marking a task complete or opening a PR:

1. Re-read the issue description or task requirements
2. Compare changes against requirements
3. Verify the build still succeeds
4. Check if documentation or ADRs need updating
5. List any gaps in the PR description
