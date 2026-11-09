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
   `genModules`, skip subdirectories named `CMakeFiles` (and other build
   artifacts: `build*`, `*.dir`, `.svn`, `.git`). The script is Python 2
   (`file()`); do not port it here. Check it is not run against the tree.
3. **Version lookup** - replace both `find_package(Subversion)` blocks with a
   `git describe --tags --always --dirty` via `find_package(Git)` +
   `execute_process(... ERROR_QUIET)`, falling back to `unknown` when git or the
   repository is absent (a tarball). Keep `gz4d_version` / `GZ4D_VERSION`
   defined in `GeoZui4D/main`; in `libgz4d/cxx` drop the unused
   `SUBVERSION_REVISION` message (verify with grep that nothing reads it). Works
   on CMake 3.14 (ADR-0001).
4. **gtest lookup** - in `libgz4d/cxx/CMakeLists.txt` change the header search
   to `gtest/gtest.h` (keeping the `find_library` calls), or use
   `find_package(GTest)` if it keeps the `GTEST_LIBS` contract simple. Remove
   the stray `message(${GTEST_LIBS})` in `gtest/CMakeLists.txt` only if it
   errors when empty.
5. **enable_testing at top level** - add `enable_testing()` to the top-level
   `CMakeLists.txt` before the `add_subdirectory` calls. This is the only
   top-level edit (owner approved); leave the existing call in `libgz4d/cxx`.
6. **Stage 1 check (fresh worktree)** - `cmake -S . -B build` and
   `cmake --build build --parallel` succeed with default options; configure
   with `-DBUILD_GTESTS=ON` succeeds; `ctest --test-dir build -N` lists
   `test_libgz4d`.

Stage 2 - make the nine gtest sources compile and pass

7. **Compile** - build `test_libgz4d` (angle, box, color, geocam, matrix,
   socket, time, timeSequence, URI; 403 lines in all). Fix include paths
   (sources use `"../include/gz4d/..."` relative includes and `<../include/...>`
   forms), missing link libs (boost, `LIBS`), and C++17 breakage, in the tests
   or `libgz4d/cxx/gtest/CMakeLists.txt` first, library headers only if the
   header is genuinely wrong.
8. **Pass** - run `ctest --test-dir build --output-on-failure`. For each
   failure decide whether the test or the library is wrong. Early reading
   suggests exact `EXPECT_EQ` on doubles (e.g. `347.7` in `angle.cpp`) will fail
   on rounding; switching to `EXPECT_DOUBLE_EQ`/`EXPECT_NEAR` is a test fix.
   A test that fails on real library behaviour is recorded in the progress
   entry and reported, not silently weakened. A test that needs the network
   (`socket.cpp`) must not make the suite flaky.
9. **Re-enable in the build script** - `scripts/build.sh`: add
   `-DBUILD_GTESTS=ON` and replace its comment block (which describes #9 as
   open) with a short accurate one. Update README build/test steps if they
   disagree with the result (check `README.md`, verify against the run).
10. **Final acceptance** - in a brand-new `git worktree` of the branch:
    `scripts/build.sh` and `ctest --test-dir build --output-on-failure`
    pass. Commit per logical change (Refs #9).

## Files to Change

| File | Change |
|------|--------|
| 19 `CMakeLists.txt` (`libgz4d/cxx/src/x3d`, `libgz4d/cxx/boost.python/*`) | delete `add_subdirectory(CMakeFiles)` |
| `libgz4d/cxx/gen_cmake.py` | skip `CMakeFiles` and build-artifact directories |
| `libgz4d/cxx/CMakeLists.txt` | drop Subversion block; gtest header `gtest/gtest.h` |
| `GeoZui4D/main/CMakeLists.txt` | version from `git describe`, fallback `unknown` |
| `CMakeLists.txt` (top level, approved) | add `enable_testing()` only |
| `libgz4d/cxx/gtest/*.cpp`, `gtest/CMakeLists.txt` | stage 2 compile/pass fixes |
| `scripts/build.sh`, `README.md` | enable `BUILD_GTESTS=ON`; correct comments and steps |

## Prior Art

The `Subversion_WC_INFO` block is replaced by the common `git describe`
idiom (`find_package(Git)`); CMake's `find_package(GTest)` is the standard
alternative to hand-rolled `find_path`, considered for step 4. Nothing new is
built.

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
| Atomic commits, issue refs | Separate commits for stubs, generator, version, gtest lookup, test fixes |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 CMake 3.14, C++17 | Yes | Only 3.14-compatible commands; no GNU extensions |
| ADR-0002 std::filesystem | No | |
| ADR-0003 Apache 2.0 | No | No new files expected; headers kept on any added |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| gtest lookup / top-level `enable_testing()` | `scripts/build.sh` flags and comments | Yes (step 9) |
| build/test behaviour | README steps | Yes (step 9) |
| `gen_cmake.py` | regenerated stubs | Yes (step 2) |
| fresh builds never run in CI | CI workflow | No - #12 |

## Open Questions

- Stage 2 size is unknown until the sources are compiled. If it needs large
  library changes (not test-side fixes), stop and raise it at plan review or a
  checkpoint instead of expanding quietly; the owner said "whole thing in this
  issue", so a split would be his call.
- Stage 1 may expose further configure or compile errors in a fresh tree beyond
  the three listed (nothing has built here before); each would be fixed in
  scope if small, or reported.

## Estimated Scope

Single PR, two stages (commits). Stage 1 small; stage 2 uncertain.
