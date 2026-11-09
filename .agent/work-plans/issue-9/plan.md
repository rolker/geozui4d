# Plan: A fresh checkout does not configure: stub CMakeFiles subdirectories, Subversion lookup, gtest header name

## Issue

https://github.com/rolker/geozui4d/issues/9

## Context

`cmake -S . -B build` fails in any fresh clone or worktree. Three defects hide
behind ignored files on the owner's main checkout: 19 tracked `CMakeLists.txt`
files end with `add_subdirectory(CMakeFiles)` (the stubs live only on the
owner's disk), `Subversion_WC_INFO` errors without a `.svn` working copy
(`libgz4d/cxx/CMakeLists.txt:398`, `GeoZui4D/main/CMakeLists.txt:270`), and
`BUILD_GTESTS=ON` looks for the non-existent `gtest/test.h`
(`libgz4d/cxx/CMakeLists.txt:432`). Also `enable_testing()` is only inside
`libgz4d/cxx`, so `ctest --test-dir build` at the top finds nothing.
Owner decisions (Checkpoint, 2026-11-09): `enable_testing()` in the top-level
`CMakeLists.txt` is approved (no other top-level change); the gtest work is
all in this issue, as two stages; CI is #12, not here.

## Approach

All verification is in a fresh tree (this worktree, or a new `git worktree`/
clone), never in `/home/roland/src/gz4d`, whose ignored stubs and
`libgz4d/.svn` mask every defect. Baseline first: confirm `cmake -S . -B build`
fails here and record the first error.

Stage 1 - configure, build, lookup, enable_testing

1. **Remove the stub references** - delete the `add_subdirectory(CMakeFiles)`
   line from the 19 files (`git grep -l 'add_subdirectory(CMakeFiles)'`:
   `libgz4d/cxx/src/x3d/` and 18 under `libgz4d/cxx/boost.python/*/`). None are
   vendored.
2. **Keep the generator from recreating them** - in `libgz4d/cxx/gen_cmake.py`
   apply the skip in BOTH `genModules` and `genBoostPython` (lines 149-213; it
   has its own subdirectory `isdir` loop at about line 160 and writes
   `add_subdirectory(sub_path)` at about line 213, and 18 of the 19 stubs come
   from it). Use one shared predicate (e.g. `skipDir(name)`) that rejects
   `CMakeFiles`, `build*`, `*.dir`, `.svn`, `.git`. The script is Python 2
   (`file()`); do not port it here. Check it is not run against the tree.
3. **Version lookup (git, not Subversion)** - replace both
   `find_package(Subversion)` blocks with `find_package(Git QUIET)` plus
   `execute_process` calls. Each call sets `WORKING_DIRECTORY
   ${CMAKE_CURRENT_SOURCE_DIR}`, `OUTPUT_STRIP_TRAILING_WHITESPACE`, `ERROR_QUIET`
   and a `RESULT_VARIABLE`, and the output is used only when the result is `0`:
   `git rev-list --count HEAD` gives the commit count, `git rev-parse --short
   HEAD` the short sha. The repository has no tags, so `git describe` alone is
   not used. Values:
   - `GeoZui4D/main/CMakeLists.txt` (block at about lines 268-279):
     `gz4d_version` becomes `4.0.<count>-g<sha>`; when git, the repository or
     either result is missing it stays `unknown`. `GZ4D_VERSION` is still
     defined from it. Line about 321: `CPACK_PACKAGE_VERSION_PATCH` becomes the
     commit count, or `0` on fallback, so `CPACK_PACKAGE_FILE_NAME`
     (`GeoZui4D_4.0-<patch>_i386`) keeps its shape.
   - `libgz4d/cxx/CMakeLists.txt` (block at about lines 396-399):
     the same lookup sets a variable `GZ4D_COMMIT_COUNT` (replacing
     `SUBVERSION_REVISION`, whose `message("revision: ...")` becomes a message of
     the new values); line about 482: `CPACK_PACKAGE_VERSION_PATCH` becomes
     `${GZ4D_COMMIT_COUNT}`, `0` on fallback. `SUBVERSION_REVISION` is not
     unused; grep for any other reader and update it. Works on CMake 3.14
     (ADR-0001).
4. **gtest lookup** - in `libgz4d/cxx/CMakeLists.txt` replace the hand-rolled
   `find_path`/`find_library` (looking for `gtest/test.h`) with
   `find_package(GTest REQUIRED)` and link `GTest::GTest` and `GTest::Main`
   (they bring pthreads, so the `GTESTS_NEED_PTHREADS` option is no longer
   needed for this). Adjust `libgz4d/cxx/gtest/CMakeLists.txt` to use the
   imported targets instead of `GTEST_LIBS`; remove its stray
   `message(${GTEST_LIBS})` only if it errors when empty.
5. **enable_testing at top level** - add `enable_testing()` to the top-level
   `CMakeLists.txt` before the `add_subdirectory` calls (a subdirectory-only
   call writes no `CTestTestfile.cmake` at the build root). This is the only
   top-level edit (owner approved); leave the existing call in `libgz4d/cxx`.
6. **Stage 1 check (fresh worktree)** - `cmake -S . -B build` and
   `cmake --build build --parallel 8` succeed with default options; configure
   with `-DBUILD_GTESTS=ON` succeeds; `cd build && ctest -N` lists
   `test_libgz4d`. (`ctest --test-dir` needs CMake 3.20, above the 3.14
   minimum, so the plan uses `cd build && ctest` everywhere.)

Stage 2 - build files and docs only; no test or library edits

The plan reviewer built `test_libgz4d` by hand from the nine unchanged sources
(angle, box, color, geocam, matrix, socket, time, timeSequence, URI) with
`g++ -std=c++17`, gtest from `libgtest-dev`, boost and `-pthread`: all compile
and all 23 tests pass, including the exact compare in `angle.cpp`. So stage 2
edits no test source and no library header. `socket.cpp` only resolves
`http://localhost`, so it needs no network. Edit a test only if the real CMake
build shows it broken; do not edit passing tests.

7. **Build and run** - build `test_libgz4d` through CMake with
   `-DBUILD_GTESTS=ON` and run `cd build && ctest --output-on-failure`. Expect
   23 tests passing. Any unexpected failure is recorded in the progress entry
   and reported, not silently weakened.
8. **Re-enable in the build script** - `scripts/build.sh`: add
   `-DBUILD_GTESTS=ON` and replace its comment block (which describes #9 as
   open) with a short accurate one. Any `ctest` call there uses `cd build &&
   ctest` unless the installed CMake supports `--test-dir` (state which in the
   commit).
9. **README** - add `libgtest-dev` to the Ubuntu package list (`README.md`
   about lines 90-106): `build.sh` now needs it, since a missing gtest is a
   `FATAL_ERROR`. Make the test step `cd build && ctest --output-on-failure`
   and fix any other build/test step that disagrees with the verified run.
10. **`.agents/README.md`** - rewrite lines about 144-160 (gtest cannot
    configure, `build.sh` leaves it off, no top-level `enable_testing()`, fresh
    clone does not configure) and about 196-205 (version from Subversion, used
    as CPack patch) to describe the fixed state: gtest on by `build.sh` and
    run with `cd build && ctest`; a fresh clone configures; the version is
    `4.0.<commit count>-g<sha>` from git with an `unknown` / patch 0 fallback.
    Verify each statement against the final code.
11. **Final acceptance** - in a brand-new `git worktree` of the branch:
    `scripts/build.sh` and `cd build && ctest --output-on-failure` pass.
    Commit per logical change (Refs #9).

## Files to Change

| File | Change |
|------|--------|
| 19 `CMakeLists.txt` (`libgz4d/cxx/src/x3d`, `libgz4d/cxx/boost.python/*`) | delete `add_subdirectory(CMakeFiles)` |
| `libgz4d/cxx/gen_cmake.py` | skip `CMakeFiles` and build-artifact dirs in both `genModules` and `genBoostPython` |
| `libgz4d/cxx/CMakeLists.txt` | git version/commit-count lookup, CPack patch = commit count (0 fallback); `find_package(GTest)` |
| `GeoZui4D/main/CMakeLists.txt` | `gz4d_version` = `4.0.<count>-g<sha>` (fallback `unknown`); CPack patch = commit count (0 fallback) |
| `CMakeLists.txt` (top level, approved) | add `enable_testing()` only |
| `libgz4d/cxx/gtest/CMakeLists.txt` | use `GTest::GTest`/`GTest::Main`; no test sources edited |
| `scripts/build.sh` | enable `BUILD_GTESTS=ON`; correct comments |
| `README.md` | `libgtest-dev` in Ubuntu packages; `cd build && ctest` test step |
| `.agents/README.md` | replace the three "still broken" descriptions (about lines 144-160, 196-205) |

## Prior Art

The `Subversion_WC_INFO` block is replaced by the common git version idiom
(`find_package(Git)` with `execute_process` running `git rev-list --count
HEAD` and `git rev-parse --short HEAD`; `git describe` is not used because the
repository has no tags). CMake's `find_package(GTest)` is the standard
alternative to hand-rolled `find_path`/`find_library` lookups for GoogleTest.

## Serves

Project goal "usable on modern systems" (`docs/roadmap.md`), Direction line
"A fresh clone configures, builds and runs its tests by hand (#9)". The
project has no design document.

## Design Picture

No change to the design picture (the project keeps no design document).

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Ask First: top-level `CMakeLists.txt` | One line, `enable_testing()`, approved by the owner on 2026-11-09; nothing else there |
| Run the build after code changes | Verified in a fresh worktree, not the main checkout |
| Never modify vendored code | None of the touched files are in Togl, BWidget or rapidxml |
| Atomic commits, issue refs | Separate commits for stubs, generator, version, gtest lookup, `enable_testing()`, build script, docs |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 CMake 3.14, C++17 | Yes | Only 3.14-compatible commands; no GNU extensions |
| ADR-0002 std::filesystem | No | |
| ADR-0003 Apache 2.0 | No | No new files expected; headers kept on any added |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| gtest lookup / top-level `enable_testing()` | `scripts/build.sh` flags and comments | Yes (step 8) |
| build/test behaviour | README steps | Yes (step 9) |
| `gen_cmake.py` | regenerated stubs | Yes (step 2, both functions) |
| version source | CPack patch in both CMakeLists, `.agents/README.md` | Yes (steps 3, 10) |
| `BUILD_GTESTS=ON` in build.sh | README prerequisites (`libgtest-dev`) | Yes (step 9) |
| fresh builds never run in CI | CI workflow | No - #12 |

## Open Questions

- The version scheme (commit count as CPack patch, `4.0.<count>-g<sha>`) was
  decided by the host and is listed for the owner's veto.
- Stage 1 may expose further configure or compile errors in a fresh tree beyond
  the three listed (nothing has built here before); each would be fixed in
  scope if small, or reported.

## Estimated Scope

Single PR, two stages (commits). Both stages small (stage 2 is build-file and docs work).
