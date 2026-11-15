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
