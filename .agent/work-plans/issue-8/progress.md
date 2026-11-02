---
issue: 8
---

# Issue #8 — Onboard the project with the workspace

## Integrated Review
**Status**: complete
**When**: 2026-11-02 17:30 -05:00
**By**: Claude Code Agent (claude-fable-5-1)

**PR**: #10 at `e2811e2`
**Sources**: 1 (Copilot R1 @ `ab29b15`, covers `e2811e2`: the two cited passages of `.agents/README.md` are unchanged since; no CI checks on the PR; no local review timeline)
**Cross-source confirmations**: 0
**CI**: pending

### Findings
- [x] (medium, Copilot R1) The guide says every start script sets `appdir` to `../lib/GeoZui4D` relative to the executable. `GeoNavScript` sets `appdir {}` (`GeoZui4D/main/MainWindow.cpp:110`), so `-nav` looks for `visualizer.tcl` in the working directory unless `GeoZui4D.conf` (read from `$GEOZUI4D`, `~/.GeoZui4D/` or the working directory, `MainWindow.cpp:57-80`) sets `appdir`. Document the `-nav` exception and the config override — `.agents/README.md`
- [x] (medium, Copilot R1) The pitfall "The version string comes from Subversion; in a git checkout it is `unknown`. Harmless." is wrong: `unknown` is used only when `svn` is not installed (`GeoZui4D/main/CMakeLists.txt:268-277`); with `svn` installed and no working copy, `Subversion_WC_INFO` fails configure, the defect issue #9 records. Replace the claim with the real behaviour and point at #9 — `.agents/README.md`

### False positives
- none

## Implementation
**Status**: complete
**When**: 2026-11-02 17:45 -05:00
**By**: Claude Code Agent (claude-fable-5-1)

**PR**: #10 at `557c9ec`
**Addressed**: Integrated Review at `e2811e2` (2026-11-02 17:30 -05:00)
**Commits**: f973b60, 557c9ec

### Actions
- [x] Startup paragraph rewritten: the four start scripts source `GeoZui4D.conf` first (from `$GEOZUI4D`, `~/.GeoZui4D/`, then the working directory), the default, `-cp` and `-wall` scripts set `appdir` to `../lib/GeoZui4D`, and `-nav` sets it empty so GeoNav3D reads `visualizer.tcl` from the working directory unless the config file sets `appdir` — `.agents/README.md:84-100` (f973b60)
- [x] Subversion pitfall rewritten: `unknown` only when `svn` is absent; with `svn` installed and no working copy `Subversion_WC_INFO` (both CMakeLists) fails configure, point 2 of #9 — `.agents/README.md` (557c9ec)

## Local Review
**Status**: complete
**When**: 2026-11-02 11:57 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: changes-requested

**PR**: #10 at `ddaef88`
**Depth**: Deep (reason: 294 changed lines, over the 200-line Deep threshold)
**Must-fix**: 1 | **Suggestions**: 5

### Findings
- [x] (must-fix) Adding -DBUILD_GTESTS=ON after #9 will not make `ctest --test-dir build` find test_libgz4d: enable_testing() is only in libgz4d/cxx/CMakeLists.txt:452, the root CMakeLists.txt has none, so root ctest reports "No tests were found" and exits 0 (probe confirmed); correct both claims and add the root enable_testing() need to #9 — `scripts/build.sh:11-16`, `.agents/README.md:141-146`
- [x] (suggestion) docs/work-plans/ is not tracked (git keeps no empty directory) and is absent from a fresh clone; say it is created on demand — `.agents/README.md:63`
- [x] (suggestion) The start scripts source every GeoZui4D.conf they find ($GEOZUI4D, ~/.GeoZui4D/, working directory; three independent ifs, MainWindow.cpp:57-80), later ones overriding earlier; not a first-found lookup — `.agents/README.md:88-91`
- [x] (suggestion) Only GeoZui4D/main/CMakeLists.txt:268-277 builds a version string with an `unknown` fallback; libgz4d/cxx/CMakeLists.txt:396-403 only sets SUBVERSION_REVISION (both still fail configure without a working copy) — `.agents/README.md:183-189`
- [x] (suggestion) "without explicit approval" for the IVS files conflicts with AGENTS.md, which lists them under Never; follow AGENTS.md or note the conflict for #7 — `.agents/README.md:175`
- [ ] (suggestion) Anchor the rule as `/worktrees/` so it cannot match a nested directory, the same trap the guide records for `build/` — `.gitignore:63`
