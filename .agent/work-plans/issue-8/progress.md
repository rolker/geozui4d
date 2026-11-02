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
- [x] (suggestion) Anchor the rule as `/worktrees/` so it cannot match a nested directory, the same trap the guide records for `build/` — `.gitignore:63`

## Implementation
**Status**: complete
**When**: 2026-11-02 12:01 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**PR**: #10 at `a71f85c`
**Addressed**: Local Review at `ddaef88` (2026-11-02 11:57 -05:00)
**Commits**: c64ae92, a71f85c, 5554023, 6b47522, 69c0171, 9c53eeb, aad1a51

### Actions
- [x] (must-fix) Root `ctest --test-dir build` finds no tests even with `BUILD_GTESTS=ON`: `enable_testing()` is only in `libgz4d/cxx/CMakeLists.txt:452`, so `build/` gets no `CTestTestfile.cmake` (re-probed with CMake 3.28: root finds none and exits 0; the subdirectory runs the test). Both comments now say so and that `test_libgz4d` is reachable only from `build/libgz4d/cxx` and below; no CMakeLists.txt edited (top level is Ask First). The missing top-level `enable_testing()` belongs to issue #9 with the gtest header fix and needs adding to #9's scope — `scripts/build.sh:11-20`, `.agents/README.md:141-153` (c64ae92, a71f85c)
- [x] (suggestion) `docs/work-plans/` is not tracked; the layout line now says it is not tracked until a plan lands and that the project's `/plan-task` writes plans there — `.agents/README.md:63` (5554023)
- [x] (suggestion) Start scripts source every `GeoZui4D.conf` found (three independent checks in `MainWindow.cpp:57-80`, later overriding earlier), not a first-found lookup — `.agents/README.md:88-93` (6b47522)
- [x] (suggestion) Version pitfall: both CMakeLists call `Subversion_WC_INFO` and set `SUBVERSION_REVISION`; only `GeoZui4D/main/CMakeLists.txt:268-277` builds `GZ4D_VERSION` with the `unknown` fallback; both fail configure without a working copy — `.agents/README.md` Common Pitfalls (69c0171)
- [x] (suggestion) IVS-copyright files now "Never modify", following AGENTS.md's Never list instead of CLAUDE.md's "without explicit approval"; the CLAUDE.md wording itself is left for issue #7 (instruction file, Ask First) — `.agents/README.md` Common Pitfalls (9c53eeb)
- [x] (suggestion) `.gitignore` rule anchored as `/worktrees/`; `git check-ignore` confirms `worktrees/<x>` still matches the tracked rule (a nested `worktrees/` is now caught only by the main checkout's untracked `.git/info/exclude`) — `.gitignore:63` (aad1a51)

Notes: the workspace's `review_progress.sh findings`/`check` select only `Integrated Review` and `Local Review (Pre-Push)`, so they could not see this post-PR `## Local Review` source; its boxes were flipped by exact-line edits in the fix commits instead. No build run: no C++ or CMake changed, and a worktree cannot configure until #9; `bash -n scripts/build.sh` passes; the repository has no pre-commit config.

## Local Review
**Status**: complete
**When**: 2026-11-02 12:04 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Dispatch**: resumed (agent a9c09e4ec432e855f, resume 1 of 3)
**Verdict**: approved

**PR**: #10 at `3ab199f`
**Depth**: Deep (reason: 348 changed lines on the whole PR diff, over the 200-line Deep threshold; fix round re-reviewed at the whole-diff tier)
**Must-fix**: 0 | **Suggestions**: 4

All six findings of the Local Review at `ddaef88` verified fixed against source: root-ctest claim corrected in both files (enable_testing() only at libgz4d/cxx/CMakeLists.txt:452, and #9 now carries the top-level enable_testing() scope); docs/work-plans/ untracked; every GeoZui4D.conf sourced, later overriding; version string only in GeoZui4D/main; IVS files follow AGENTS.md's Never; `.gitignore` anchored as `/worktrees/` (git check-ignore confirms).

### Findings
- [ ] (suggestion) `gz4d_x3d` is built only if PNG, TIFF and JPEG are all found (libgz4d/cxx/src/x3d/CMakeLists.txt:4), and GeoZui4D cannot link without it; say so — `.agents/README.md:26`
- [ ] (suggestion) MissionPlan (and MissionPlanDisplayTypes) are not built: objects/CMakeLists.txt adds only dxf, GeoData, nodeTypes, ogr, S57, Sonar; drop it or mark "present but not built" — `.agents/README.md:50`
- [ ] (suggestion) Tcl extension scripts install to lib/GeoZui4D/scripts (GeoZui4D/main/scripts/CMakeLists.txt:4), not lib/GeoZui4D — `.agents/README.md:57`
- [ ] (suggestion) GLUT is optional (find_package(GLUT) without REQUIRED, linked only if found, GeoZui4D/main/CMakeLists.txt:138-141); mark it so in the link list — `.agents/README.md:78-79`

## Integrated Review
**Status**: complete
**When**: 2026-11-02 12:06 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**PR**: #10 at `0b1cf85`
**Sources**: 2 (Copilot R1 @ `ab29b15`, both comments closed by the Integrated Review at `e2811e2` and re-verified fixed at this head; Local Review @ `3ab199f` (covers `0b1cf85` via bookkeeping), four open suggestions; no human review requested; the repository has no CI at all, no workflows and no check-runs on the PR)
**Cross-source confirmations**: 0
**CI**: pending

Copilot R1, re-checked against `0b1cf85`: the `-nav` `appdir` exception and the config override are documented (`.agents/README.md:86-100`; `MainWindow.cpp` sets `appdir {}` in `GeoNavScript` and appends the shared part that sources `GeoZui4D.conf` after it, so a config file can override), and the "Harmless" Subversion claim is replaced (`.agents/README.md:192-200`, matching `GeoZui4D/main/CMakeLists.txt:268-277` and `libgz4d/cxx/CMakeLists.txt:396-403`). Not re-listed. Copilot has not run on any later push.

### Findings
- [x] (suggestion, Local Review @ `3ab199f`) `gz4d_x3d` is built only when PNG, TIFF and JPEG are all found (`libgz4d/cxx/src/x3d/CMakeLists.txt:4`, else "skipping gz4d_x3d"); the top level then still hands `GeoZui4D/main` the name `gz4d_x3d` (`CMakeLists.txt:17`), so the app cannot link. The guide states the library unconditionally (line 26) and says a missing optional library only "skips the programs that need it" (line 176). Say that the three image libraries are required for the app — `.agents/README.md`
- [x] (suggestion, Local Review @ `3ab199f`) `objects/` lists MissionPlan as a scene object type, but `GeoZui4D/main/objects/CMakeLists.txt` adds only dxf, GeoData, nodeTypes, ogr, S57, Sonar, and `LINK_OBJECT(MissionPlan)` is commented out (`engine/Scene3D.cpp:96`). Its header is still included (`engine/Scene3D.cpp:12`), so do not drop it: mark it "present, header included, sources not built" (MissionPlanDisplayTypes likewise is not built) — `.agents/README.md:50`
- [ ] (suggestion, Local Review @ `3ab199f`) Tcl extension scripts install to `lib/GeoZui4D/scripts` (`GeoZui4D/main/scripts/CMakeLists.txt:4`), not `lib/GeoZui4D` — `.agents/README.md:57`

### False positives
- (Local Review @ `3ab199f`) GLUT is optional and the link list should mark it so (`GeoZui4D/main/CMakeLists.txt:138-141`, `find_package(GLUT)` without `REQUIRED`) — the CMake lookup is optional, but the program is not: `main.cpp:12` includes `<GL/glut.h>` unconditionally and `main.cpp:68` calls `glutInit`, and nine more sources under `GeoZui4D/main` use GLUT, so a build without it fails at compile or link. The guide's "links ... GLUT" and "initialises GLUT" are accurate; calling it optional would mislead. (At most, a missing GLUT surfaces at build time rather than configure; not worth a box.)

## Checkpoint
**Status**: complete
**When**: 2026-11-02 12:09 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: owner
**After**: findings
**Decision**: address

Owner's answer to the findings checkpoint on PR #10 (Integrated Review at `486458b`, three open suggestions on `.agents/README.md`: the three image libraries are required for the app, MissionPlan is present but not built, Tcl extension scripts install to `lib/GeoZui4D/scripts`): "address" — fix them, re-review and re-triage before the merge checkpoint.
