# Plan: Declare build dependencies in a runnable, CI-tested form; fix the netCDF C++ package name

## Issue

https://github.com/rolker/geozui4d/issues/16

## Context

#18 delivered items 1 and 2 (`scripts/install_deps.sh`, the `libnetcdf-c++4-dev` name). The owner's scope for what remains: guard the Kinect object with `GZ4D_KINECT` so OpenNI is optional again (done first, verified by a build without OpenNI); fetch GoogleTest with `FetchContent` when it is not installed; print a configure summary of optional features, report only (a missing optional library never fails the configure or CI). Item 4 (CI that runs the script in a clean `ubuntu:24.04` job) goes to #12. Adding `libxi-dev`, `libexif-dev`, `libflac++-dev`, `festival-dev` goes to a separate issue the host opens.

Today `GeoZui4D/main/CMakeLists.txt:184-191` finds OpenNI and defines `GZ4D_KINECT` when found, but nothing tests that define: `engine/Scene3D.cpp:9` includes `objects/Kinect.h` (which includes `<gz4d/Kinect.h>` and so `XnCppWrapper.h`), `Scene3D.cpp:118` has `LINK_OBJECT(Kinect)`, and `objects/CMakeLists.txt:32` always compiles `Kinect.cpp`. So a machine without OpenNI cannot build the app. `libgz4d/cxx/CMakeLists.txt:349-358` already sets `ni_FOUND` and `test/CMakeLists.txt:393` already guards `gzKinect` on it, so the library side needs no guard, only a switch to force it off.

## Approach

1. **Add a switch to force OpenNI off** in `GeoZui4D/main/CMakeLists.txt`: `option(GZ4D_WITH_KINECT "Use OpenNI for the Kinect object when found" ON)`. The find block (`:184-191`) runs only when ON; it sets `ni_FOUND` (TRUE/FALSE) and, when found, `add_definitions(-DGZ4D_KINECT)`. With OFF the build is the same as on a machine without OpenNI, which is how the guard is verified here (Ubuntu 24.04 with `libopenni-dev` installed). Declare the same option in `libgz4d/cxx/CMakeLists.txt:349-358` (configured first, top-level `CMakeLists.txt:14`) so one `-DGZ4D_WITH_KINECT=OFF` also skips `gzKinect` in the library; the option is a cache variable, so the top-level file is not touched.
2. **Guard the application code**: wrap the include (`engine/Scene3D.cpp:9`) and `LINK_OBJECT(Kinect)` (`:118`) in `#ifdef GZ4D_KINECT`; in `objects/CMakeLists.txt:32` add `Kinect.cpp` only `if(ni_FOUND)` (`ni_FOUND` is set at `main/CMakeLists.txt:184`, before `add_subdirectory(objects)` at `:204`, so it is visible there). The only script that creates a Kinect object is `GeoZui4D/main/gzmod/nosb.py:19` (`gz newobject Kinect ...`); no `.tcl` script does. It needs no code change and fails at run time with OpenNI off; the PR description says so.
3. **Verify** by building twice in scratch build directories: `-DGZ4D_WITH_KINECT=OFF` (must configure, compile and link, with no `XnCppWrapper.h` reached and no `Kinect` object compiled) and the default (still defines `GZ4D_KINECT` and builds `Kinect.cpp`). The durable test afterwards is #12's clean `ubuntu:24.04` job: once `libopenni-dev` leaves `install_deps.sh`, it builds without OpenNI on every run. The PR description records this for #12. Commit this step on its own with step 4.
4. **Package list and docs for the guard**: delete the `libopenni-dev` line from `PACKAGES` in `scripts/install_deps.sh` and correct its comment about `GZ4D_KINECT`. No `OPTIONAL_PACKAGES` array and no `--optional` flag (host decision; #19 adds one if it needs it). README.md Prerequisites (~lines 90-102) says that installing `libopenni-dev` enables the Kinect object and `-DGZ4D_WITH_KINECT=OFF` turns it off. `.agents/README.md:114` (options list) gains `GZ4D_WITH_KINECT`; correct any OpenNI-required wording found there (verify against the text first).
5. **GoogleTest by FetchContent** in `libgz4d/cxx/CMakeLists.txt:449-461`, inside the existing `if(BUILD_GTESTS)` block so a default configure never touches the network: keep `find_package(GTest)` first (no REQUIRED); when not found, `include(FetchContent)`, `FetchContent_Declare(googletest URL <v1.14.0 release tarball> URL_HASH SHA256=...)`, `set(INSTALL_GTEST OFF)`, `set(BUILD_GMOCK OFF)` (the suite links only `GTest::GTest` and `GTest::Main`, `gtest/CMakeLists.txt:14`), `FetchContent_MakeAvailable(googletest)`, then `add_library(GTest::GTest ALIAS gtest)` and `add_library(GTest::Main ALIAS gtest_main)`. Pin v1.14.0 because it is the release Ubuntu 24.04's `libgtest-dev` (1.14.0-1) ships, so the installed and the fetched path test the same GoogleTest; it also satisfies ADR-0001 (v1.14.0 through v1.16.0 ask for CMake 3.13; only v1.17.0 raises it to 3.16 and C++17). The comment next to the pin says this. The library sets `BUILD_SHARED_LIBS` ON (`:12-16`), so the fetched gtest builds shared; fine for `ctest` from the build tree. The fetch needs network access at configure; the failure message says so. Verify with the installed package (default path) and with `-DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON` (fetch path), running `ctest` in both. `libgtest-dev` stays in `install_deps.sh` as the offline path, comment changed.
6. **GoogleTest wording**: correct `scripts/build.sh` header comment ("configure stops if it is missing"), README.md `libgtest-dev` paragraph, and `.agents/README.md:149-157` (no longer `find_package(GTest REQUIRED)`; a missing GoogleTest is fetched; the `GTest_DIR` / `GTEST_ROOT` notes apply only to the installed path).
7. **Configure summary**, report only, app only (`GeoZui4D/main/CMakeLists.txt`, next to `message("version: ...")`), no `FATAL_ERROR`. Two parts via a helper `gz4d_report_feature(name flag)`: "Optional features (build works without them)" lists only features checked against the source to survive absence. Already confirmed: FFmpeg (`GZ4D_AV`, `:107-110`), GLEW (`GZ4D_GLEW`, `:144-149`), OpenNI/Kinect (guarded in step 2). To check before listing: Bluetooth (`engine/Wiimote.h` is included by `main.cpp:33` and `Scene3D.cpp:65`; see whether it needs `bluetooth/bluetooth.h`), Jasper, JPEG/TIFF/PNG (`.agents/README.md:185` says all three are required in practice), X11, jack. Whatever fails the check is dropped. A second line group, "Required in practice, found", prints Vorbis (`objects/SoundOgg.cpp` always compiled), PortAudio (`Sound.cpp`/`Sound.h` include its headers unconditionally), NetCDF and NetCDF C++ (`engine/Scene3D.h` reaches `<ncAtt.h>`; `.agents/README.md:189`), GLUT, and says `MISSING` for any not found (the build then fails later, as today). The Vorbis and Bluetooth blocks set no `*_FOUND` variable; add one. Existing per-block "not found" messages stay. No library-side summary and no new dependency lookups.
8. **Remove the build skill** (owner: "let's remove that skill"): `git rm -r .claude/skills/build/`; delete the `/build` row from the CLAUDE.md Skills table (`:86`), nothing else in CLAUDE.md; reword `.agents/README.md:177` (".claude/skills/build/ is tracked") to match, after reading the sentence it sits in. Move the skill's content that nothing else covers into `.agents/README.md` near its build section (`:138`): "Run the full build, with the tests, as part of any audit of this project"; report warnings even on successful builds; do not modify a `CMakeLists.txt` to fix a build error without asking; `scripts/build.sh --parallel <N>` on a machine short of memory (the script passes its arguments to the build step, the last `--parallel` wins). Skip what the README already says (verify each against `scripts/build.sh` and the README text). Check that `grep -rn "skills/build\|/build"` finds no remaining reference.
9. **Review items**: run the full default build (`scripts/build.sh`) and `ctest` before the push; confirm the summary prints both with everything found and in the OpenNI-off build.

## Files to Change

| File | Change |
|------|--------|
| `GeoZui4D/main/CMakeLists.txt` | `GZ4D_WITH_KINECT` option, `ni_FOUND`, `*_FOUND` for Vorbis and Bluetooth, configure summary |
| `GeoZui4D/main/engine/Scene3D.cpp` | `#ifdef GZ4D_KINECT` around include and `LINK_OBJECT(Kinect)` |
| `GeoZui4D/main/objects/CMakeLists.txt` | `Kinect.cpp` only when OpenNI found |
| `libgz4d/cxx/CMakeLists.txt` | same option for the OpenNI find; GoogleTest `FetchContent` fallback inside `if(BUILD_GTESTS)` |
| `scripts/install_deps.sh` | delete `libopenni-dev`; comments (OpenNI, GoogleTest) |
| `scripts/build.sh` | header comment on GoogleTest |
| `README.md` | OpenNI optional and how to enable or disable; GoogleTest wording |
| `.agents/README.md` | `:114` options, `:149-157` GoogleTest, `:177` skill reference, build guidance moved from the skill |
| `CLAUDE.md` | delete the `/build` row of the Skills table only (owner approved) |
| `.claude/skills/build/SKILL.md` | deleted (owner approved) |

## Prior Art

The existing `GZ4D_KINECT` define and the `ni_FOUND` guard on `gzKinect` in `libgz4d/cxx/test/CMakeLists.txt` are reused rather than a new mechanism. `GeoZui4D/main/CMakeLists.txt` already prints per-dependency "not found" messages; the summary collects the flags they set. For GoogleTest, the upstream-documented `FetchContent` pattern with `find_package` first is used. The build skill's guidance is folded into `.agents/README.md` rather than kept as a second place. No CI is built here (#12 owns it).

## Serves

None (the project states no goals); it serves "builds from a fresh clone". Design document: none.

## Design Picture

No change to the design picture; the project has no design document.

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Only what's needed | One package list stays in `install_deps.sh`; no second list in CI; no new dependency lookups. |
| Document from source | README, `.agents/README.md`, `install_deps.sh` and `build.sh` wording corrected against the code after the change. |
| Put each thing at the level it applies to | CI left to #12; extra packages left to #19. |
| Test what breaks | The OFF build is checked once here; #12's clean no-OpenNI job is the durable test. |
| Verify, do not assume | "Optional" proven by a build with the switch off, not by reading the code. |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 CMake 3.14, C++17 | Yes | `FetchContent_MakeAvailable` is 3.14; v1.14.0 asks for CMake 3.13, so the pin is compliant. |
| ADR-0002 std::filesystem | No | |
| ADR-0003 Apache 2.0 | No | Fetched GoogleTest (BSD-3) is test-only and not distributed. |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| `libopenni-dev` leaves required list | `install_deps.sh` comment, README, `.agents/README.md:114` | Yes |
| Build skill deleted | CLAUDE.md Skills row, `.agents/README.md:177`, guidance moved to `.agents/README.md` | Yes |
| GoogleTest fallback | `scripts/build.sh` comment, README, `.agents/README.md:149-157` | Yes |
| `gzmod/nosb.py` creates a Kinect object | Fails at run time with OpenNI off; noted in the PR | Yes |
| Top-level `CMakeLists.txt` (Ask First) | Not touched; the `-D` option is a cache variable that reaches both sub-projects | Yes |
| CI workflow in a clean container | #12 | No, follow-up (#12) |
| libxi, libexif, libflac++, festival packages | New issue opened by the host | No, follow-up |

Build-impact note for the PR (CMakeLists changes): no required dependency is added; OpenNI becomes truly optional; GoogleTest can be fetched at configure time (network needed only when it is not installed); default behavior on a machine with every package is unchanged.

## Open Questions

- None open. Decided: option name `GZ4D_WITH_KINECT`, GoogleTest pin v1.14.0, no `--optional` flag, the build skill removed.

## Estimated Scope

Single PR, four commits: Kinect guard with the package-list change, GoogleTest fetch, configure summary, build skill removal.
