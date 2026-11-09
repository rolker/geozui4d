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
