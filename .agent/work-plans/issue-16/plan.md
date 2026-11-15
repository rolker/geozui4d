# Plan: Declare build dependencies in a runnable, CI-tested form; fix the netCDF C++ package name

## Issue

https://github.com/rolker/geozui4d/issues/16

## Context

#18 delivered items 1 and 2 (`scripts/install_deps.sh`, the `libnetcdf-c++4-dev` name). The owner's scope for what remains: guard the Kinect object with `GZ4D_KINECT` so OpenNI is optional again (done first, verified by a build without OpenNI); fetch GoogleTest with `FetchContent` when it is not installed; print a configure summary of optional features, report only (a missing optional library never fails the configure or CI). Item 4 (CI that runs the script in a clean `ubuntu:24.04` job) goes to #12. Adding `libxi-dev`, `libexif-dev`, `libflac++-dev`, `festival-dev` goes to a separate issue the host opens.

Today `GeoZui4D/main/CMakeLists.txt:184-191` finds OpenNI and defines `GZ4D_KINECT` when found, but nothing tests that define: `engine/Scene3D.cpp:9` includes `objects/Kinect.h` (which includes `<gz4d/Kinect.h>` and so `XnCppWrapper.h`), `Scene3D.cpp:118` has `LINK_OBJECT(Kinect)`, and `objects/CMakeLists.txt:32` always compiles `Kinect.cpp`. So a machine without OpenNI cannot build the app. `libgz4d/cxx/CMakeLists.txt:349-358` already sets `ni_FOUND` and `test/CMakeLists.txt:393` already guards `gzKinect` on it, so the library side needs no guard, only a switch to force it off.

## Approach

1. **Add a switch to force OpenNI off** in `GeoZui4D/main/CMakeLists.txt`: `option(GZ4D_WITH_KINECT "Use OpenNI for the Kinect object when found" ON)`. The find block runs only when ON; it sets `ni_FOUND` and, when found, `add_definitions(-DGZ4D_KINECT)`. With OFF the build is the same as on a machine without OpenNI, which is how the guard is verified here (Ubuntu 24.04 with `libopenni-dev` installed). Mirror the same option in `libgz4d/cxx/CMakeLists.txt:349-358` so `-DGZ4D_WITH_KINECT=OFF` from the top level also skips `gzKinect` in the library (the option is a cache variable, so one `-D` reaches both).
2. **Guard the application code**: wrap the include (`engine/Scene3D.cpp:9`) and `LINK_OBJECT(Kinect)` (`:118`) in `#ifdef GZ4D_KINECT`; in `objects/CMakeLists.txt` add `Kinect.cpp` to the source list only `if(ni_FOUND)` (the variable set by the main CMakeLists before `add_subdirectory(objects)`; check the order, and if `objects/` is processed first, use `list(APPEND ...)` in the parent instead). Grep for any other `Kinect` use (the earlier grep shows only these) and for Tcl scripts that create a `Kinect` object, which would fail at run time with a clear "unknown object" message rather than at build time.
3. **Verify** by building twice in scratch build directories: `-DGZ4D_WITH_KINECT=OFF` (must configure, compile and link; check `nm`/`strings` or that no `Kinect` symbol and no `XnCppWrapper` include is reached) and the default (still defines `GZ4D_KINECT` and builds Kinect.cpp). Commit this step on its own.
4. **Update the package list**: move `libopenni-dev` out of `PACKAGES` in `scripts/install_deps.sh` into an `OPTIONAL_PACKAGES` array that `--list` prints under a clear marker only with a new `--optional` flag (default behavior stays required-only, so the single list stays one file); fix its comment. Update README.md (lines ~90-102) to say OpenNI is optional and how to turn it off. Update `.agents/README.md` lines 185-186 if they claim OpenNI is required, and the file inventory note about Kinect if any (verify first).
5. **GoogleTest by FetchContent** in `libgz4d/cxx/CMakeLists.txt:449-461`: keep `find_package(GTest)` first (no REQUIRED); when not found, `include(FetchContent)`, `FetchContent_Declare(googletest URL <release tarball> URL_HASH SHA256=...)` pinned to a release (v1.14.0 supports C++14 and CMake 3.13+, so it fits ADR-0001), `set(INSTALL_GTEST OFF)`, `FetchContent_MakeAvailable(googletest)`, then `add_library(GTest::GTest ALIAS gtest)` and `add_library(GTest::Main ALIAS gtest_main)` so `gtest/CMakeLists.txt:14` (`GTest::GTest GTest::Main`) is unchanged. Fetch needs network access at configure; when the fetch fails the error says so. Verify with the installed package (default path) and with `-DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON` (fetch path), running `ctest` in both. Then `libgtest-dev` stays in `install_deps.sh` as the offline path, with its comment changed to say the build fetches GoogleTest when it is absent; the `scripts/build.sh` header comment ("configure stops if it is missing") and README line 101 are corrected.
6. **Configure summary**, report only: at the end of `GeoZui4D/main/CMakeLists.txt` (next to `message("version: ...")`), print one block, "Optional features:", one line per feature as `on`/`off (not found)` for Vorbis, Bluetooth, NetCDF, NetCDF C++, OpenNI/Kinect, FFmpeg (avformat), PortAudio, GLEW, JPEG/TIFF/PNG/Jasper, X11, from the `*_FOUND` variables each find block already sets (a few blocks, e.g. Kinect's here, only set a define; add the variable). A small helper function `gz4d_report_feature(name flag)` keeps it to one line each. No `FATAL_ERROR` is added. Leave the existing per-block "not found" messages. Do the same for the library's optional finds only if they already set `*_FOUND`; do not add new dependency lookups.
7. **Review items**: run the full default build (`scripts/build.sh`) and `ctest` before the push; confirm the summary prints both when everything is found and in the OpenNI-off build.

## Files to Change

| File | Change |
|------|--------|
| `GeoZui4D/main/CMakeLists.txt` | `GZ4D_WITH_KINECT` option, `ni_FOUND`, configure summary |
| `GeoZui4D/main/engine/Scene3D.cpp` | `#ifdef GZ4D_KINECT` around include and `LINK_OBJECT(Kinect)` |
| `GeoZui4D/main/objects/CMakeLists.txt` | `Kinect.cpp` only when OpenNI found |
| `libgz4d/cxx/CMakeLists.txt` | same option for the OpenNI find; GoogleTest `FetchContent` fallback |
| `scripts/install_deps.sh` | `libopenni-dev` to an optional list; comments |
| `scripts/build.sh`, `README.md`, `.agents/README.md` | GoogleTest and OpenNI wording |

## Prior Art

The existing `GZ4D_KINECT` define and the `ni_FOUND` guard on `gzKinect` in `libgz4d/cxx/test/CMakeLists.txt` are reused rather than a new mechanism. `GeoZui4D/main/CMakeLists.txt` already prints per-dependency "not found" messages; the summary collects the flags they set. For GoogleTest, the upstream-documented `FetchContent` pattern with `find_package` first is used. No CI is built here (#12 owns it).

## Serves

None (the project states no goals); it serves "builds from a fresh clone". Design document: none.

## Design Picture

No change to the design picture; the project has no design document.

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Only what's needed | One package list stays in `install_deps.sh`; no second list in CI; no new dependency lookups. |
| Document from source | README, `.agents/README.md`, `install_deps.sh` and `build.sh` wording corrected against the code after the change. |
| Put each thing at the level it applies to | CI left to #12; extra packages left to their own issue. |
| Verify, do not assume | "Optional" proven by a build with the switch off, not by reading the code. |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 CMake 3.14, C++17 | Yes | `FetchContent_MakeAvailable` is 3.14; a GoogleTest release that builds with C++17 and CMake 3.14 is pinned. |
| ADR-0002 std::filesystem | No | |
| ADR-0003 Apache 2.0 | No | Fetched GoogleTest (BSD-3) is test-only and not distributed. |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| `libopenni-dev` leaves required list | `install_deps.sh` comment, README, `.agents/README.md` | Yes |
| GoogleTest fallback | `scripts/build.sh` comment, README line 101 | Yes |
| Top-level `CMakeLists.txt` (Ask First) | Not touched; the `-D` option is a cache variable that reaches both sub-projects | Yes |
| CI workflow in a clean container | #12 | No, follow-up (#12) |
| libxi, libexif, libflac++, festival packages | New issue opened by the host | No, follow-up |

Build-impact note for the PR (CMakeLists changes): no required dependency is added; OpenNI becomes truly optional; GoogleTest can be fetched at configure time (network needed only when it is not installed); default behavior on a machine with every package is unchanged.

## Open Questions

- Option name `GZ4D_WITH_KINECT` (default ON, use if found) is a proposal; the owner may prefer another name.
- Pin the GoogleTest release (v1.14.0 proposed) and its tarball hash; a later release needs C++14 and CMake 3.16, above the project's 3.14 minimum.
- Should `install_deps.sh --optional` exist now, given that the follow-up issue adds more optional packages? Proposed: yes, so that issue only adds names.

## Estimated Scope

Single PR, three commits: Kinect guard with the package-list change, GoogleTest fetch, configure summary.
