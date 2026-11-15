---
issue: 16
---

# Issue #16 — Declare build dependencies in a runnable, CI-tested form; fix the netCDF C++ package name

## Issue Review
**Status**: complete
**When**: 2026-11-15 09:25 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #16

### Scope Assessment

**Well-scoped?** Mostly. #18 delivered items 1 and 2 (option a: `scripts/install_deps.sh`, `libnetcdf-c++4-dev`, plus `libxmu-dev` and `libopenni-dev`). What remains is four pieces: (3) GoogleTest via `FetchContent` (`libgz4d/cxx/CMakeLists.txt:454` is still `find_package(GTest REQUIRED)`); (4) CI, which is #12's workflow and should run `scripts/install_deps.sh --update --yes` in a clean `ubuntu:24.04` job; (5) a configure summary of optional features, with the Kinect guard (`GZ4D_KINECT` around the include in `engine/Scene3D.cpp`, the `LINK_OBJECT(Kinect)`, and the Kinect source in `objects/CMakeLists.txt`) so `libopenni-dev` can leave the required list. One PR could do 3 and 5 (with the Kinect guard); item 4 belongs with #12. Adding more packages (XInput, exif, FLAC++, Festival) one at a time is open-ended and would be better as a separate follow-up issue.
**Right repo?** Yes, the project repo. Item 3 touches the library's CMakeLists.txt and item 5 the application's `GeoZui4D/main/CMakeLists.txt`; neither is the top-level file, so no "ask first" gate applies.
**Serves**: none written down (the project states no goals); it serves "builds from a fresh clone". Design document: none.
**Design picture?** No change.
**Dependencies**: #12 (CI workflow) for item 4. Item 4 needs the script, which exists. #2 (retiring Tcl/Tk) will change the package list later.

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Only what's needed | Watch | The README claims "one command installs every build and test dependency"; keep the script the single list, and avoid a second list in the CI file. |
| Keep one current design | OK | No design change. |
| Verify against source | OK | The issue's claims about GZ4D_KINECT and Scene3D.cpp match the source (the define exists at `GeoZui4D/main/CMakeLists.txt:186`). |
| Put each thing at the level it applies to | Action needed | Decide who owns the CI test of the package list: item 4 is #12's work. Mark item 4 as delivered by #12 and do not build a workflow here, or the two issues will conflict. |
| Document from source | Watch | `install_deps.sh` has a comment saying GZ4D_KINECT is never tested; the comment and the `libopenni-dev` entry need to change with the Kinect guard. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| ADR-0001 CMake 3.14 and C++17 | Yes | `FetchContent_MakeAvailable` needs CMake 3.14, so it is allowed; `FetchContent_Declare` with a URL needs no newer version. |
| ADR-0002 std::filesystem | No | |
| ADR-0003 Apache 2.0 | No | Fetched GoogleTest is BSD-3, used only for tests. |

### Consequences

- The Kinect guard changes `libgz4d` too: `libgz4d/cxx/CMakeLists.txt:350` finds OpenNI without defining `GZ4D_KINECT`, and `gz4d/Kinect.h` includes `XnCppWrapper.h`. Check whether the library headers are included unconditionally.
- Remove `libopenni-dev` from `scripts/install_deps.sh` when the guard lands, and correct its comment.
- If GoogleTest is fetched, `libgtest-dev` can leave the list (the fetch needs network access; keep `find_package` first, as the issue says).
- A configure summary should be printed once, at the end of `GeoZui4D/main/CMakeLists.txt` (it has 8 "not found" messages).
- The README says to run the script; update it if the list changes.

### Recommendations

- Close items 1 and 2 as done in #18, and treat the rest as the issue's scope: Kinect guard, GoogleTest `FetchContent`, configure summary. Hand item 4 to #12.
- Owner decision, scope: should a missing optional dependency fail CI or only be reported? The issue leaves this open. Suggested default: report only, since the build must work without them after the Kinect guard.
- Do the Kinect guard first and verify that a build without OpenNI compiles (remove the package or force `ni_FOUND` off), because that is the real test of "optional".
- Open a separate issue for adding the remaining optional packages (XInput, exif, FLAC++, Festival), one per build, rather than extending this one.

### Actions
- [ ] Decide who owns the CI test of the package list: item 4 is #12's work. Mark item 4 as delivered by #12 and do not build a workflow here, or the two issues will conflict.
- [ ] Close items 1 and 2 as done in #18, and treat the rest as the issue's scope: Kinect guard, GoogleTest `FetchContent`, configure summary. Hand item 4 to #12.
- [ ] Owner decision, scope: should a missing optional dependency fail CI or only be reported? The issue leaves this open. Suggested default: report only, since the build must work without them after the Kinect guard.
- [ ] Do the Kinect guard first and verify that a build without OpenNI compiles (remove the package or force `ni_FOUND` off), because that is the real test of "optional".
- [ ] Open a separate issue for adding the remaining optional packages (XInput, exif, FLAC++, Festival), one per build, rather than extending this one.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:28 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: owner
**After**: issue-actions
**Decision**: proceed

The owner's answers, to three questions:
- Q1 "Should #16 hand item 4 to #12 and cover only F3?" (F3: guard the Kinect object with GZ4D_KINECT so OpenNI becomes optional again, done first and verified by a build without OpenNI; fetch GoogleTest with FetchContent; print a configure summary of optional features): "Yes, hand item 4 to #12".
- Q2 "Should a missing optional library fail CI, or only be reported in the summary?": "Report only".
- Q3 "Open a separate issue for adding those optional packages (libxi-dev, libexif-dev, libflac++-dev, festival-dev), instead of doing it in #16?": "Yes, open a separate issue".

Scope for the plan: the Kinect guard (first), GoogleTest by FetchContent, the configure summary (report only). Items 1 and 2 were delivered by #18; item 4 goes to #12; the optional packages go to their own issue.

## Plan Authored
**Status**: complete
**When**: 2026-11-15 09:29 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-16/plan.md` at `d60dbdf`

Guard the Kinect object with GZ4D_KINECT first, with a GZ4D_WITH_KINECT switch to verify a build without OpenNI, then fetch GoogleTest when not installed and print a report-only configure summary. CI goes to #12; extra packages to a new issue.

## Plan Review
**Status**: complete
**When**: 2026-11-15 09:33 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: needs-work

**Issue**: #16 — Declare build dependencies in a runnable, CI-tested form; fix the netCDF C++ package name
**Plan**: `.agent/work-plans/issue-16/plan.md` at `d60dbdf`
**Branch**: `feature/issue-16`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | Three commits in one PR: the Kinect guard, the GoogleTest fetch, the configure summary. It matches the owner's Checkpoint: item 4 goes to #12 and the optional packages go to #19. Step 6's "do the same for the library's optional finds" is open-ended (see finding 6). |
| Issue alignment | Needs work | Serves: "None (the project states no goals); it serves 'builds from a fresh clone'. Design document: none." That is acceptable for a project. Every review-issue finding is addressed. The summary would label several libraries "optional" that the build cannot do without (finding 1). |
| File targeting | Needs work | Checked against the source: `ni_FOUND` is set at `GeoZui4D/main/CMakeLists.txt:184`, before `add_subdirectory(objects)` at `:204`, so the plan's "check the order" holds and `objects/CMakeLists.txt` can test `ni_FOUND` directly. The library is configured first (top-level `CMakeLists.txt:14`), so the option declared there is the one that takes effect. Files the plan misses: `.agents/README.md:149-157` (the GTest passage) and `:114` (options list); `.claude/skills/build/SKILL.md:21-22`; `GeoZui4D/main/gzmod/nosb.py:19` (findings 3 and 4). The ".agents/README.md lines 185-186" reference does not match anything about OpenNI. |
| Consequences | Needs work | Missing: the build skill's "GoogleTest must be installed" text (Ask First, since it is a skill file), and the `.agents/README.md` GTest passage. |
| Principle alignment | Needs work | "Only what's needed": the `OPTIONAL_PACKAGES` array and the `--optional` flag are not needed now (host decision; finding 2). "Document from source": the summary's feature list does not match the source (finding 1). "Test what breaks": the OFF build is a one-time check, and the plan should name what keeps it tested (finding 7). |
| ADR compliance | Needs work | ADR-0001: `FetchContent_MakeAvailable` is available in 3.14, and v1.14.0's CMakeLists has `cmake_minimum_required(VERSION 3.13)`, so the pin is compliant. The plan's reason for the pin is wrong, though (finding 5). ADR-0003: BSD-3, test-only, fine. |
| ROS conventions | N/A | Not a ROS project. |

### Findings

1. **[Principle alignment / Document from source]** Step 6 would print Vorbis, NetCDF C++ and PortAudio as optional features that show `off (not found)`, but the build fails without each of them. `.agents/README.md:189` already records that vorbisfile, GLUT and NetCDF C++ are required in practice (`objects/SoundOgg.cpp` is always compiled; `engine/Scene3D.h` reaches `<ncAtt.h>`). `objects/Sound.cpp` and `Sound.h` include PortAudio headers unconditionally as well. A summary that calls these "optional" misleads the people and the CI it is written for. Resolution: list only the features whose absence the build survives. Checked so far: FFmpeg (`GZ4D_AV`), GLEW (`GZ4D_GLEW`), OpenNI/Kinect once guarded, and probably Bluetooth and Jasper (the app links them but includes none of their headers). Check each one against the source before listing it. Leave the required-in-practice ones out, or print them separately as "required, found". Separately, the Vorbis and Bluetooth blocks set no `*_FOUND` variable either, not only Kinect's.
2. **[Principle alignment / host decision]** Step 4's `OPTIONAL_PACKAGES` array and `--optional` flag go against the host's decision (no `--optional` now; #19 adds it if it needs it). The guard does not need either one. The smallest form is to delete the `libopenni-dev` line from `PACKAGES` and to say in README.md (Prerequisites) that installing `libopenni-dev` enables the Kinect object and that `-DGZ4D_WITH_KINECT=OFF` turns it off. No new array and no new flag. Update the plan's Open Questions to match.
3. **[Consequences]** The GoogleTest fallback makes three more texts wrong: `.agents/README.md:149-157` ("found with `find_package(GTest REQUIRED)` … a missing GoogleTest stops the configure", plus the `GTest_DIR` and `GTEST_ROOT` notes, which apply only to the installed path); `.claude/skills/build/SKILL.md:21-22` ("GoogleTest must be installed"), which is a skill file and so needs owner approval under AGENTS.md "Ask First"; and `.agents/README.md:114`, which lists the options and should gain `GZ4D_WITH_KINECT`. Add all three to Files to Change and the Consequences table, and flag the skill edit for approval.
4. **[File targeting]** `GeoZui4D/main/gzmod/nosb.py:19` runs `gz newobject Kinect …`. It is the one script that creates a Kinect object (no `.tcl` script does). With OpenNI off it fails at run time. No code change is needed, but step 2's grep should cover it and the PR description should say so.
5. **[ADR compliance / host decision check]** The plan says a later release "needs C++14 and CMake 3.16", and that is wrong. Checked upstream: the top-level `CMakeLists.txt` asks for `cmake_minimum_required(VERSION 3.13)` in v1.14.0, v1.15.0, v1.15.2 and v1.16.0. Only v1.17.0 raises it to 3.16, and v1.17.0 also requires C++17. The host's pin of v1.14.0 is still right, because it matches Ubuntu 24.04's installed `libgtest-dev` 1.14.0-1 and so both paths test the same GoogleTest. Correct the reason in the plan and in any comment next to the pin.
6. **[Scope]** In step 6, "Do the same for the library's optional finds only if they already set `*_FOUND`" leaves the scope open, because `libgz4d/cxx/CMakeLists.txt` has many such blocks. The issue review said the summary should be printed once, at the end of `GeoZui4D/main/CMakeLists.txt`. Suggestion: limit it to the app, or name the library features up front.
7. **[Test what breaks]** The OFF build in step 3 checks the guard once. Say what keeps it tested from then on: once `libopenni-dev` leaves `install_deps.sh`, #12's clean `ubuntu:24.04` job builds without OpenNI on every run. Record that as the durable test, in the plan and in the PR, so #12 knows. Suggestion.
8. **[Approach]** Suggestions for step 5: keep the fetch inside the existing `if(BUILD_GTESTS)` block (`libgz4d/cxx/CMakeLists.txt:451`), so a default configure never touches the network. Set `BUILD_GMOCK OFF` along with `INSTALL_GTEST OFF`; the suite links only `GTest::GTest` and `GTest::Main` (`gtest/CMakeLists.txt:14`), and v1.14.0 builds gmock by default. Also note that the library sets `BUILD_SHARED_LIBS` ON (`libgz4d/cxx/CMakeLists.txt:12-16`), so the fetched gtest builds as a shared library. That is fine for `ctest` from the build tree.

Host decisions checked: the `GZ4D_WITH_KINECT` name is fine, so keep it. The v1.14.0 pin is right, but the plan's reason for it is wrong (finding 5). No `--optional` flag is correct, and the plan's step 4 has to change to match (finding 2).

### Summary

The plan is sound: the Kinect guard comes first and is verified by a build with OpenNI off, and GoogleTest is fetched only when it is not installed. It needs four corrections before implementation. The summary's feature list must match what is really optional; step 4 must drop the optional array and flag; the GoogleTest documents (one of them a skill file, which needs approval) must be added; and the reason for the GoogleTest pin must be fixed.

### Recommended Actions

- [ ] Limit the configure summary to features whose absence the build survives, each checked against the source, or print the required-in-practice ones (Vorbis, GLUT, NetCDF C++, PortAudio) separately as required.
- [ ] Replace step 4's `OPTIONAL_PACKAGES` and `--optional` with deleting the `libopenni-dev` line, plus README wording; update Open Questions.
- [ ] Add `.agents/README.md:114` and `:149-157` and `.claude/skills/build/SKILL.md:21-22` to Files to Change and Consequences, and mark the skill edit "Ask First".
- [ ] Correct the GoogleTest version reason: v1.14.0 to v1.16.0 need CMake 3.13 and only v1.17.0 needs 3.16 and C++17; keep the v1.14.0 pin because it matches Ubuntu 24.04.
- [ ] Note `gzmod/nosb.py` as the one script that creates a Kinect object.
- [ ] (Suggestion) Keep the summary to the app; name #12's no-OpenNI CI build as the durable test; keep the fetch inside `if(BUILD_GTESTS)` and set `BUILD_GMOCK OFF`.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:39 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: owner
**After**: plan
**Decision**: revise

The Plan Review at 08b59f7 (needs-work) has four must-fix findings; the revision addresses all four and takes its suggestions. Asked whether #16 may edit the project's build skill (.claude/skills/build/SKILL.md, whose "GoogleTest must be installed" line the GoogleTest fallback makes wrong), the owner asked where it came from and whether the workspace needs it, then answered: "let's remove that skill."

So the revised plan also deletes .claude/skills/build/ and updates what names it: the CLAUDE.md Skills table row for /build, and .agents/README.md:177 ("so .claude/skills/build/ is tracked"). The skill's one project rule that is not elsewhere, "Run the full build, with the tests, as part of any audit of this project", moves to .agents/README.md, with its other guidance (report warnings; do not modify a CMakeLists.txt to fix a build error without asking; scripts/build.sh --parallel <N> on a machine short of memory) where it is not already covered.

## Plan Authored
**Status**: complete
**When**: 2026-11-15 09:40 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-16/plan.md` at `40874b9`

Revision after the plan review: the configure summary lists only features the build survives without (required-in-practice ones print separately), the libopenni-dev line is deleted with no --optional flag, GoogleTest docs and the pin reason are corrected, and the build skill is removed with its guidance moved to .agents/README.md (owner decision).

## Plan Review
**Status**: complete
**When**: 2026-11-15 09:44 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: ready

**Issue**: #16 — Declare build dependencies in a runnable, CI-tested form; fix the netCDF C++ package name
**Plan**: `.agent/work-plans/issue-16/plan.md` at `40874b9`
**Branch**: `feature/issue-16`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | One PR, four commits: Kinect guard with the package-list change, GoogleTest fetch, configure summary, build-skill removal. The summary is now limited to the app (earlier finding 6 taken). The skill removal is the owner's decision from the latest Checkpoint ("let's remove that skill") and stays small: the skill folder, one CLAUDE.md row, `.agents/README.md`. |
| Issue alignment | Good | Serves: "None (the project states no goals); it serves 'builds from a fresh clone'. Design document: none." That is acceptable for a project. All five must-fix items from the first review are addressed: (1) the summary has two groups, optional and "required in practice", each to be checked against the source; (2) no `OPTIONAL_PACKAGES` and no `--optional`, just the deleted `libopenni-dev` line plus README wording; (3) `.agents/README.md:114` and `:149-157` are in Files to Change, and the skill text is handled by deleting the skill, which the owner approved; (4) the GoogleTest pin reason is corrected (v1.14.0 to v1.16.0 ask for 3.13, v1.17.0 for 3.16 and C++17); (5) `gzmod/nosb.py:19` is noted. |
| File targeting | Good | Checked: CLAUDE.md:86 is the `/build` row; `.agents/README.md:177` is the `.claude/skills/build/` sentence; `:114` is the options list; `:149-157` is the GTest passage; `:185` is the PNG/TIFF/JPEG row. Two references are off: NetCDF C++ is cited as `.agents/README.md:189`, but the row is `:186` (`:189` is under Common Pitfalls). Step 4's "correct any OpenNI-required wording" in `.agents/README.md` finds nothing: the only Kinect mention is the module list at `:77`, which is neutral. The earlier "lines 185-186 about OpenNI" reference is gone from the plan. |
| Consequences | Good | The consequences table covers the skill removal, the GoogleTest docs, the OpenNI package-list change, `nosb.py`, and the top-level CMakeLists.txt being left alone. In the workspace, `skills/build` appears only in a test fixture (`test_project_integration_audit.sh`) and in another issue's plan, not as a dependency on this project's skill. |
| Principle alignment | Good | "Only what's needed" holds now that the array and flag are gone. "Test what breaks": the one-time OFF build, with #12's clean job as the durable test, is recorded. "Document from source": one open check, Bluetooth, resolves the other way from what the plan expects (finding 1). |
| ADR compliance | Good | ADR-0001: `FetchContent_MakeAvailable` is 3.14, and v1.14.0 asks for 3.13. ADR-0003: GoogleTest is BSD-3, test-only. ADR-0002 is not triggered. |
| ROS conventions | N/A | Not a ROS project. |

### Findings

1. **[Principle alignment / Document from source]** (must-fix during implementation; no plan revision needed) Bluetooth is not optional on Linux. `main.cpp:33` and `engine/Scene3D.cpp:65` include `engine/Wiimote.h`, which on non-WIN32 includes `<gz4d/WiimoteManager.h>`. That reaches `gz4d/socket/BTSocketManager.h:11-13` (`<bluetooth/bluetooth.h>`, `hci.h`, `hci_lib.h`) and `BTSocket.h:16`. `engine/CMakeLists.txt:42` always compiles `Wiimote.cpp`. A missing `libbluetooth-dev` therefore fails the compile. The same applies to JPEG, TIFF and PNG through `gz4d_x3d` (`.agents/README.md:185`). Step 7 says "Whatever fails the check is dropped". Instead, these should be listed under "Required in practice, found" (with the `bluetooth_FOUND` the plan already adds), and `.agents/README.md:186` should name Bluetooth among the app's required-in-practice libraries. Jasper and jack have no headers included by the app or its included gz4d headers, so they look optional; X11 is still to be checked (Togl on Linux).
2. **[File targeting]** (suggestion) The NetCDF C++ reference `.agents/README.md:189` should be `:186`. Also drop "correct any OpenNI-required wording" for `.agents/README.md`, since there is none, or keep it as a check that is expected to find nothing.
3. **[Approach]** (suggestion) Step 8's final check, `grep -rn "skills/build\|/build"`, also matches `.gitignore:2` (`/build/`), README's `scripts/build.sh` lines and kdevelop files, so it cannot "find no remaining reference". Use `grep -rn 'skills/build\|`/build`'` (excluding `.agent/work-plans/`) instead.
4. **[Approach]** (suggestion) README.md:101-102 ("`libgtest-dev` is needed only for the tests") is in step 6. With the fetch, it is needed only for offline test builds; word it the way the `install_deps.sh` comment will be worded.

### Summary

The revision addresses every finding of the first review and follows the owner's decisions (item 4 to #12, report only, optional packages to #19, build skill removed). One fact from the open checks needs carrying into implementation: Bluetooth, like JPEG/TIFF/PNG, is required in practice and belongs in the summary's required group, not dropped. The plan is ready for implementation.

### Recommended Actions

- [ ] List Bluetooth (and JPEG, TIFF, PNG) under "Required in practice, found" in the configure summary, and add Bluetooth to `.agents/README.md:186`.
- [ ] (Suggestion) Correct the `.agents/README.md:189` reference to `:186`; narrow step 8's grep; align the README `libgtest-dev` wording with the fetch fallback.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:43 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (second plan review verdict ready; its one must-fix is an implementation instruction, no plan revision needed)
**After**: plan
**Decision**: proceed

The Plan Review at 7393c15 is ready. Its findings go to the implement pass: list Bluetooth, JPEG, TIFF and PNG under "Required in practice, found" instead of dropping them, and name Bluetooth at .agents/README.md:186 (Wiimote.cpp is always compiled and reaches <bluetooth/bluetooth.h>); cite :186, not :189, for NetCDF C++; narrow the step 8 grep to `skills/build` or the literal `/build` command; reword README.md:101-102 so libgtest-dev is needed only for offline test builds.

## Implementation
**Status**: complete
**When**: 2026-11-15 10:44 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Branch**: feature/issue-16 at `6b5995e`

The earlier pass made the plan's four commits; this pass checked them against the plan and the second plan review's findings, found nothing missing, made no new code commits, and ran the verification.

Commits:
- `abd79d1` Guard the Kinect object so OpenNI is optional: `GZ4D_WITH_KINECT` option (default ON) in `GeoZui4D/main/CMakeLists.txt` and `libgz4d/cxx/CMakeLists.txt`; `ni_FOUND` set in the app; `#ifdef GZ4D_KINECT` around the include and `LINK_OBJECT(Kinect)` in `engine/Scene3D.cpp`; `objects/Kinect.cpp` compiled only `if(ni_FOUND)`; `libopenni-dev` removed from `scripts/install_deps.sh` (no optional array, no `--optional` flag); README and `.agents/README.md` options list updated.
- `c19b962` Fetch GoogleTest when it is not installed: inside `if(BUILD_GTESTS)`, `find_package(GTest)` first, else `FetchContent` of v1.14.0 (URL plus SHA256, `INSTALL_GTEST OFF`, `BUILD_GMOCK OFF`) aliased to `GTest::GTest` and `GTest::Main`; the pin comment gives the corrected reason (Ubuntu 24.04's 1.14.0-1; v1.14.0 to v1.16.0 ask for CMake 3.13). `scripts/build.sh`, `install_deps.sh`, README.md (libgtest-dev needed only for offline test builds) and `.agents/README.md` GTest passage reworded.
- `6d4006d` Configure summary at the end of `GeoZui4D/main/CMakeLists.txt`, report only: optional features FFmpeg, GLEW, OpenNI, Jasper; "required in practice" JPEG, TIFF, PNG, PortAudio, JACK, Vorbis, Bluetooth, GLUT, NetCDF, NetCDF C++, X11, `MISSING` when not found. `*_FOUND` added for FFmpeg, JACK, Vorbis, Bluetooth. `.agents/README.md:186` (not :189) now names Bluetooth, PortAudio, JACK and X11 with the includes that make them required. Rechecked this pass: Bluetooth through `engine/Wiimote.h` and `BTSocketManager.h`, JACK and PortAudio through `objects/AudioSpectrum.h` (Jack only on non-WIN32), X11 through vendored `Togl/togl.c` (`<X11/Xlib.h>`).
- `6b5995e` Remove the project's build skill (owner decision): `.claude/skills/build/` deleted, only the `/build` row removed from CLAUDE.md, `.agents/README.md` `.gitignore` note reworded, and the skill's remaining guidance (full build with tests in audits, report warnings, ask before changing a `CMakeLists.txt` to fix a build, `scripts/build.sh --parallel <N>`) moved to `.agents/README.md`'s build section.

Verification (clean scratch build directories, 6 jobs):
- Default (`-DBUILD_GTESTS=ON`, installed GoogleTest found at `/usr/lib/x86_64-linux-gnu/cmake/GTest`): configure, full build and link succeeded (about 12 min); `GZ4D_KINECT` is in the app's compile flags and `objects/Kinect.cpp` and `test/gzKinect` were built; summary printed every required library `found`, FFmpeg, GLEW and OpenNI `found`, Jasper `off (not found)`. `ctest`: 1/1 passed (`test_libgz4d`). 463 compiler warnings, from unchanged code (e.g. `URI.h:276` no return, Boost bind deprecation, OpenNI deprecated overload); none come from this branch's changes.
- Kinect off (`-DGZ4D_WITH_KINECT=OFF`, OpenNI installed): full build and link succeeded; configure printed `skipping gzKinect.cpp` and `OpenNI Kinect object (GZ4D_KINECT): off (GZ4D_WITH_KINECT=OFF)`; no `GZ4D_KINECT` in the app flags, no Kinect source compiled and no `XnCppWrapper.h` reached anywhere in the log; no `gzKinect` executable.
- GoogleTest fetched (`-DBUILD_GTESTS=ON -DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON`): configure printed the fetch message and downloaded v1.14.0 (network available); gtest and gtest_main built as shared libraries, no gmock; `test_libgz4d` built; `ctest`: 1/1 passed.
- Narrowed grep `grep -rn 'skills/build\|`/build`'` (excluding `.git` and `.agent/work-plans/`): no matches.

Not changed, for the PR description: `GeoZui4D/main/gzmod/nosb.py:19` creates a Kinect object and fails at run time with OpenNI off; #12's clean `ubuntu:24.04` job is the durable no-OpenNI test; the top-level `CMakeLists.txt` was not touched. Scratch builds and the worktree's gitignored `build/` were removed.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-15 12:09 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: changes-requested

**Branch**: feature/issue-16 at `d0665b2`
**Base**: origin/main
**Depth**: Deep (reason: 12 files, 560 lines changed incl. work plan; touches CLAUDE.md and a skill file)
**Must-fix**: 1 | **Suggestions**: 2
**Round**: 1 | **Ship**: continue — round 1: 1 must-fix; first round always re-reviews after fixes

### Findings
- [x] (must-fix) GLEW is listed as an optional feature ("build works without them"), but `objects/Flow.cpp` is always compiled and calls `glewInit()`, `glewIsSupported()` and `glMultiDrawArraysEXT()` unguarded, while `<GL/glew.h>` is reached only under `GZ4D_GLEW` (`gz4d/glo.h:12-14`); without GLEW the build fails. Move GLEW to "required in practice" in the summary and in the `.agents/README.md` Dependencies row (or guard Flow.cpp, which is wider scope) — `GeoZui4D/main/CMakeLists.txt:332`, `.agents/README.md:204`, `GeoZui4D/main/objects/Flow.cpp:33-35,114` (found by Codex; confirmed by reading the source)
- [x] (suggestion) CMake 3.24+ prints a CMP0135 developer warning on the GoogleTest fetch path (URL download with no `DOWNLOAD_EXTRACT_TIMESTAMP`); add `if(POLICY CMP0135) cmake_policy(SET CMP0135 NEW) endif()` before `FetchContent_Declare`, which keeps CMake 3.14 working — `libgz4d/cxx/CMakeLists.txt:472`
- [x] (suggestion) The `if(NOT TARGET gtest)` FATAL_ERROR is almost never reached: a failed download or hash mismatch stops configure inside `FetchContent_MakeAvailable` with CMake's own error. Print the "install libgtest-dev or configure with network access" hint before the fetch, or reword — `libgz4d/cxx/CMakeLists.txt:479-482` (Gemini and Claude adversarial)

Outside reviewers: Gemini (agy) completed, 1 suggestion (above). Codex completed, 1 must-fix (GLEW, above). Claude adversarial (fresh subagent) completed: no must-fix, 2 suggestions (above); it judged GLEW guarded through `glo.h` and missed Flow.cpp's unguarded calls. Static analysis: shellcheck not installed; `bash -n` clean on both changed scripts; no linter profile for CMake or Markdown.

## Implementation
**Status**: complete
**When**: 2026-11-15 12:14 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**Branch**: feature/issue-16 at `4ceb250`
**Addressed**: Local Review (Pre-Push) at `d0665b2` (2026-11-15 12:09 -05:00)
**Commits**: d338c74, daa6720, 4ceb250

### Actions
- [x] (must-fix) GLEW moved from the optional features to "required in practice" in the configure summary, and added to the `.agents/README.md` Dependencies row with the `objects/Flow.cpp` evidence; Flow.cpp itself not guarded (out of scope) — `GeoZui4D/main/CMakeLists.txt:346`, `.agents/README.md:204`
- [x] (suggestion) `cmake_policy(SET CMP0135 NEW)` under `if(POLICY CMP0135)` before `FetchContent_Declare`; the fetched configure on CMake 3.28 printed one CMP0135 warning before and none after — `libgz4d/cxx/CMakeLists.txt:475`
- [x] (suggestion) The libgtest-dev hint now goes in the message before the fetch; the unreachable `if(NOT TARGET gtest)` FATAL_ERROR is removed — `libgz4d/cxx/CMakeLists.txt:470`

### Verification
- Plain configure (`cmake -S . -B <scratch>`) succeeds; summary lists GLEW under "required in practice".
- `-DBUILD_GTESTS=ON -DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON` configure fetches GoogleTest with no CMP0135 warning and prints the new hint.
- No full build: the changes touch configure messages and a FetchContent policy only, nothing compiled. Scratch build dirs removed.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-15 12:17 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: approved

**Branch**: feature/issue-16 at `161e57c`
**Base**: origin/main
**Depth**: Deep (reason: 12 files, 604 lines changed incl. work plan; touches CLAUDE.md and a skill file; fix round classified on the whole diff)
**Must-fix**: 0 | **Suggestions**: 1
**Round**: 2 | **Ship**: recommended — no must-fix findings; round 1's must-fix (GLEW) and both suggestions are fixed

### Findings
- [ ] (suggestion) The "required in practice" summary is incomplete: OpenGL (`find_package(OpenGL)` without REQUIRED, line 167, needed by the GL code and Togl) is not listed, and the X11 line checks only `X11_FOUND` although `Togl/togl.c:34` includes `<X11/Xmu/StdCmap.h>`, which `FindX11` reports separately (`X11_Xmu_FOUND`); add `OPENGL_FOUND` and check Xmu, and name OpenGL in the `.agents/README.md` Dependencies row — `GeoZui4D/main/CMakeLists.txt:338-350`, `.agents/README.md:204` (Codex: Xmu; Claude adversarial: OpenGL; confirmed by reading the source)

Round-1 fixes checked: GLEW is under "required in practice" (`GeoZui4D/main/CMakeLists.txt:346`) and in the `.agents/README.md` row with the Flow.cpp evidence; `cmake_policy(SET CMP0135 NEW)` under `if(POLICY CMP0135)` precedes `FetchContent_Declare`; the unreachable FATAL_ERROR is gone and the libgtest-dev hint prints before the fetch.

Outside reviewers: Gemini (agy) completed, no issues. Codex completed, 1 suggestion (Xmu, above). Claude adversarial (fresh subagent) completed, no must-fix; suggestions OpenGL (above) and a pre-existing `ctest --test-dir` inconsistency at `.agents/README.md:134` (line not touched by this branch; dropped). Static analysis: shellcheck not installed; `bash -n` clean on both changed scripts; no linter profile for CMake or Markdown. Verified: `cmake -DGZ4D_WITH_KINECT=OFF -DBUILD_GTESTS=ON` configures cleanly in a scratch dir, prints the summary with Kinect off, skips gzKinect, no Kinect source in any generated build.make; only dev warnings are pre-existing (PythonLibs policy, line 33 syntax warning).
