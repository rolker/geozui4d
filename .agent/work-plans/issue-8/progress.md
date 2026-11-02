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
