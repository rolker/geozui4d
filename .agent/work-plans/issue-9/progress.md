---
issue: 9
---

# Issue #9 — A fresh checkout does not configure

## Issue Review
**Status**: complete
**When**: 2026-11-09 10:01 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #9

### Scope Assessment

**Well-scoped?** Yes, with a watch. The three defects (stub `add_subdirectory(CMakeFiles)` lines, the Subversion lookup, the gtest header name) plus the added top-level `enable_testing()` all serve one outcome: a fresh clone configures, builds and tests. Verified in the worktree: 19 files hold `add_subdirectory(CMakeFiles)`; `Subversion_WC_INFO` is at `libgz4d/cxx/CMakeLists.txt:398` and `GeoZui4D/main/CMakeLists.txt:270`; `gtest/test.h` is at `libgz4d/cxx/CMakeLists.txt:432`; `enable_testing()` is only at line 452. The risk is point 3: "make `test_libgz4d` pass" is open-ended, because the nine gtest sources are not known to compile. One PR is fine for points 1, 2 and the lookup fix; getting the tests to compile and pass may belong in a follow-up issue.
**Right repo?** Yes. All the changes are in GeoZui4D build files.
**Serves**: the project goal "usable on modern systems" and the Direction line "A fresh clone configures, builds and runs its tests by hand (#9)" in `docs/roadmap.md`. It also gates the other Direction lines (#1, #2, #3, #6), per the roadmap's deferral. The project has no separate design document.
**Design picture?** No change.
**Dependencies**: none blocking. #8 is merged. It blocks re-enabling `BUILD_GTESTS=ON` in `scripts/build.sh`, and every modernization issue per the roadmap deferral.

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Project rules (`AGENTS.md`): ask first before changing top-level `CMakeLists.txt` | Action needed | The `enable_testing()` addition touches the top-level `CMakeLists.txt`, an Ask First item. Get the owner's explicit approval before editing it. The issue comment records the need, not an approval. |
| Project rules: run the build after code changes | Watch | Acceptance needs a fresh worktree configure and build, not only the main checkout, since the main checkout masks the defects. |
| Project rules: never modify vendored code | Watch | Confirm none of the 19 CMake files sit in vendored directories (Togl, BWidget, rapidxml). The listed paths are libgz4d only, so it looks fine. |
| Project goals | OK | Directly serves the acceptance case. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| ADR-0001 (CMake 3.14, C++17) | Yes | Fixes must work on CMake 3.14+. `git describe` and `find_package(GTest)` are fine. |
| ADR-0002 (std::filesystem) | No | |
| ADR-0003 (Apache 2.0) | No | Only relevant if new files are added; check license headers on any. |

### Consequences

- `scripts/build.sh` should re-enable `BUILD_GTESTS=ON` (stated in the acceptance section).
- The README build and test steps should be checked against the fixed behaviour.
- `libgz4d/cxx/gen_cmake.py` must skip `CMakeFiles` and other build-artifact directories so the stubs are not regenerated.
- Adding a CI build workflow would stop this class of failure from recurring. It is not in scope.

### Recommendations

- Ask the owner for explicit approval to edit the top-level `CMakeLists.txt` (`enable_testing()`) before implementation, as `AGENTS.md` requires.
- Split point 3 into two stages in the plan: (a) fix the header lookup and `enable_testing()`, then (b) make the nine gtest sources compile and pass. If (b) is large, open a follow-up issue and keep acceptance line 1 on the build.
- Verify with a fresh `git worktree` configure and build as the acceptance line says, not in the main checkout.
- Consider a follow-up issue for a CI build workflow, since no fresh build has ever run.

### Actions
- [ ] The `enable_testing()` addition touches the top-level `CMakeLists.txt`, an Ask First item. Get the owner's explicit approval before editing it. The issue comment records the need, not an approval.
- [ ] Ask the owner for explicit approval to edit the top-level `CMakeLists.txt` (`enable_testing()`) before implementation, as `AGENTS.md` requires.
- [ ] Split point 3 into two stages in the plan: (a) fix the header lookup and `enable_testing()`, then (b) make the nine gtest sources compile and pass. If (b) is large, open a follow-up issue and keep acceptance line 1 on the build.
- [ ] Verify with a fresh `git worktree` configure and build as the acceptance line says, not in the main checkout.
- [ ] Consider a follow-up issue for a CI build workflow, since no fresh build has ever run.

## Checkpoint
**Status**: complete
**When**: 2026-11-09 10:06 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: owner
**After**: issue-actions
**Decision**: proceed

Owner's answers at the issue-actions checkpoint (AskUserQuestion, 2026-11-09), to the three questions the Issue Review raised:

- Ask First, top-level `CMakeLists.txt`: "Yes, allow it". The plan and the implementation may add `enable_testing()` to the top-level `CMakeLists.txt`; no other top-level change without asking again.
- Scope of point 3 (gtest): "Whole thing in this issue". Fix the header lookup and make `test_libgz4d` build and pass here; if the tests need large code changes, the plan review raises it again.
- CI build workflow follow-up: "Yes, open the issue". Opened as #12 (Add a CI build workflow so a fresh clone is configured and built on every push); not part of this drive.

The review's other two actions (verify in a fresh worktree, not the main checkout; split point 3 into lookup then tests in the plan) are mechanism and go into the plan-task handoff.

## Plan Authored
**Status**: complete
**When**: 2026-11-09 10:07 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-9/plan.md` at `c51d659`

Two stages: (1) remove stub CMakeFiles lines, git-describe version, gtest/gtest.h lookup and top-level enable_testing(), verified in a fresh worktree; (2) make the nine gtest sources compile and pass, with build.sh and README updated. Stage 2 size is unknown.

## Plan Review
**Status**: complete
**When**: 2026-11-09 10:12 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: needs-work

**Issue**: #9 — A fresh checkout does not configure: stub CMakeFiles subdirectories, Subversion lookup, gtest header name
**Plan**: `.agent/work-plans/issue-9/plan.md` at `c51d659`
**Branch**: `feature/issue-9`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | One PR, two stages. Stage 2 is small: built by hand in the scratchpad (`g++ -std=c++17 -I libgz4d/cxx/include`, `-lgtest -lgtest_main`, boost date_time/system/program_options/thread, `-pthread`), all nine sources compile with no change and all 23 tests pass (GCC, gtest from `libgtest-dev`). No library changes are needed; the Checkpoint's "large changes" trigger does not fire. |
| Issue alignment | Good | All three defects, the `enable_testing()` addition, and both acceptance lines are covered. The Issue Review's actions (fresh worktree, two stages, owner approval, CI follow-up) are addressed. Serves: Project goal "usable on modern systems" (`docs/roadmap.md`), Direction line "A fresh clone configures, builds and runs its tests by hand (#9)". The project has no design document. Checked: both strings are in `docs/roadmap.md` (lines 5, 29). |
| File targeting | Needs work | `gen_cmake.py` fix names the wrong function for 18 of 19 stubs (finding 1). `.agents/README.md` is missing (finding 4). All 19 `add_subdirectory(CMakeFiles)` files are under `libgz4d/cxx/src/x3d` and `libgz4d/cxx/boost.python/`; none is vendored (checked with `git grep -l`). |
| Consequences | Needs work | CPack reads `SUBVERSION_REVISION` (finding 2); `BUILD_GTESTS=ON` in `scripts/build.sh` makes `libgtest-dev` a hard prerequisite the README does not list (finding 3); `.agents/README.md` documents all three defects as current (finding 4). |
| Principle alignment | Good | Only `enable_testing()` at top level (owner-approved); no other top-level edit. Verification in a fresh worktree (this worktree has no `CMakeFiles/` or `.svn` directories outside `build/`). No vendored or IVS files touched. Prior art stated. Design picture: none kept. |
| ADR compliance | Good | ADR-0001: `find_package(Git)`, `execute_process`, `find_package(GTest)` are all available in CMake 3.14. ADR-0002/0003 not triggered. |
| ROS conventions | N/A | Not a ROS project. |

Checked against source: a top-level `enable_testing()` is needed. A minimal 3.14-minimum project with `enable_testing()`/`add_test()` only in a subdirectory writes no `CTestTestfile.cmake` at the build root, and `ctest --test-dir b -N` reports 0 tests. With `enable_testing()` added at the top level it reports 1 (CMake 3.28.3). `socket.cpp` only resolves `http://localhost`, so it needs no network.

### Findings

1. **[File targeting] must-fix**: Step 2 changes only `genModules` in `gen_cmake.py`, but 18 of the 19 stubs are under `boost.python/` and come from `genBoostPython` (lines 149-213), which has its own `isdir` loop at line 160 and writes `add_subdirectory(sub_path)` at line 213. Apply the skip to both functions, ideally with one shared predicate.
2. **[Consequences] must-fix**: Step 3 says `SUBVERSION_REVISION` is unused in `libgz4d/cxx`, but `libgz4d/cxx/CMakeLists.txt:482` and `GeoZui4D/main/CMakeLists.txt:321` set `CPACK_PACKAGE_VERSION_PATCH` from it, and both feed `CPACK_PACKAGE_FILE_NAME`. The plan must say what the CPack patch becomes in both files, for example the `git rev-list --count HEAD` result or `0`. It must also say what form `gz4d_version` takes. The repository has no tags, so `git describe --tags --always` gives a bare sha (`acad6a8`) where it used to give `4.0.<rev>`. Something like `4.0-g<sha>`, falling back to `4.0-unknown`, keeps the old shape.
3. **[Consequences] must-fix**: Step 9 adds `-DBUILD_GTESTS=ON` to `scripts/build.sh`, and a missing gtest is a `FATAL_ERROR`. So the registered build command fails on any machine without `libgtest-dev`, which the README's Ubuntu prerequisite list (`README.md:90-106`) does not include. Add `libgtest-dev` to that list explicitly. "Update README if it disagrees" does not cover the prerequisites.
4. **[Consequences] must-fix**: `.agents/README.md` describes the defects this plan fixes as current: lines 144-160 (gtest cannot configure, `build.sh` leaves it off, no top-level `enable_testing()`, fresh clone does not configure) and lines 196-205 (the version comes from Subversion and is used as the CPack patch). Add it to Files to Change.
5. **[Issue alignment] suggestion**: The acceptance and README test commands use `ctest --test-dir`, which arrived in CMake 3.20, while the project minimum is 3.14 (ADR-0001). The README has no Ubuntu version, and Ubuntu 20.04 ships 3.16. The issue's acceptance line is fine to keep, but the README's test step should use `cd build && ctest --output-on-failure` or state that `--test-dir` needs CMake 3.20 or later.
6. **[Approach] suggestion**: In the `git describe` block, check `RESULT_VARIABLE` (not only empty output) and set `WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}`, so a tarball or a git-less machine gets the fallback cleanly. `build/` is in `.gitignore`, so `--dirty` will not fire on build output.
7. **[Approach] suggestion**: Prefer `find_package(GTest)` in step 4. Its imported target brings Threads along, which removes the need for the `GTESTS_NEED_PTHREADS` option (default OFF), and `libgtest.a` needs pthread on older glibc.
8. **[Approach] suggestion**: Step 8's expectation that the exact `EXPECT_EQ` on `347.7` in `angle.cpp` will fail did not hold: it passes. Do not change tests that pass. Edit only what the real CMake build actually shows to be broken.

### Summary

The structure is sound, the scope fits in one PR, and stage 2 is a build-file change, not a library rewrite: all 23 tests compile and pass as they stand. Four corrections are needed before implementation. They are the generator function, the CPack and version consequences, the `libgtest-dev` prerequisite and `.agents/README.md`. Each is a small plan edit.

### Recommended Actions

- [ ] Extend the `gen_cmake.py` skip to `genBoostPython` as well as `genModules`.
- [ ] State the new `gz4d_version` form and what `CPACK_PACKAGE_VERSION_PATCH` becomes in both CMakeLists files; drop the claim that `SUBVERSION_REVISION` is unused.
- [ ] Add `libgtest-dev` to the README's Ubuntu prerequisite list in step 9.
- [ ] Add `.agents/README.md` (lines 144-160, 196-205) to Files to Change.
- [ ] Optional: make the README test step work on CMake older than 3.20; check `RESULT_VARIABLE` in the git block; prefer `find_package(GTest)`; leave passing tests unchanged.

## Checkpoint
**Status**: complete
**When**: 2026-11-09 10:10 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: host (plan review verdict needs-work with four must-fix corrections to the plan text, none structural and none changing scope; /orchestrator's table lets the host answer revise at plan)
**After**: plan
**Decision**: revise

The Plan Review (465ab4c) found four must-fix corrections and four suggestions, all mechanism. The reviewer also built test_libgz4d by hand from the nine unchanged sources: all 23 tests compiled and passed, so stage 2 needs no library changes and nothing goes back to the owner on scope.

Host decision on must-fix 2 (what replaces the Subversion revision), listed for the owner's veto: the version patch becomes the commit count (`git rev-list --count HEAD`), an integer that grows like the old revision did, so CPack's `4.0.<patch>` keeps its shape; `gz4d_version` becomes `4.0.<count>-g<short sha>` from `git describe --always`; when git is missing or the tree is not a repository, both fall back to the existing `unknown` and patch 0, checking RESULT_VARIABLE and setting WORKING_DIRECTORY (suggestion 6). The repository has no tags, so a bare `git describe` is not usable on its own.

The plan-task re-run carries the four must-fix items (skip CMakeFiles in both genModules and genBoostPython of gen_cmake.py; the version scheme above; libgtest-dev added to the README's Ubuntu package list; .agents/README.md lines 144-160 and 196-205 added to Files to Change) and the four suggestions (ctest step compatible with CMake 3.14, `cd build && ctest`; the git describe fallback above; find_package(GTest); do not edit tests that already pass).
