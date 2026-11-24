---
issue: 25
---

# Issue #25 — Build on Ubuntu 26.04 without breaking 24.04, with install_deps.sh aware of each platform's requirements

## Implementation
**Status**: complete
**When**: 2026-11-24 11:24 to 11:45 -05:00
**By**: Claude Code Agent (claude-fable-5-2)

Worked directly in the worktree, without the review loop: no issue review,
no plan, no plan review, no code review (the owner's light-level trial, see
Trial counts below).

### What was done

Found by building on Ubuntu 26.04.1 (CMake 4.2.3, GCC 15.2.0, Boost 1.90.0,
GDAL 3.12.2, Tcl/Tk 8.6.16) until `scripts/build.sh` passed:

1. **Boost.System.** Boost 1.90 has no compiled `boost_system` and no CMake
   component for it. Both `libgz4d/cxx/CMakeLists.txt` and
   `GeoZui4D/main/CMakeLists.txt` now ask for `system` under
   `OPTIONAL_COMPONENTS`, so it is linked where it exists and skipped where
   it does not.
2. **GDAL 3.12 const pointers.** `OGRFeature::GetDefnRef()` and
   `GetFieldDefnRef()` return const pointers there. In
   `libgz4d/cxx/include/gz4d/ogr/Feature.h` the field definition wrapper
   takes a const pointer (it only reads), and the feature definition
   wrapper, which takes a counted reference, gets a `const_cast`.
3. **Vendored Togl and GCC 15.** `togl.c` passes non-const Tk structures
   where Tk 8.6 declares const ones; GCC 14 and later make that an error.
   `GeoZui4D/main/CMakeLists.txt` adds
   `-Wno-error=incompatible-pointer-types` to the `gz_togl` target. The
   vendored source is untouched.
4. **Boost.Python bindings.** With Boost 1.90's own CMake files the `python`
   component is found next to Python 3.14, and the Python 2 bindings then
   fail to compile (`Pixel.cpp`) and to link (`libboost_python27.so` does
   not exist). They are now built only when the Python found is Python 2.
   `GZ4D_PYTHON` keeps its default.
5. **`scripts/install_deps.sh`.** Reads `ID` and `VERSION_ID` from
   `/etc/os-release`; `RELEASE_ADD` and `RELEASE_DROP` hold what a release
   adds to or drops from the common list. Known releases: `ubuntu:24.04` and
   `ubuntu:26.04`, both empty, because the 24 common packages all have an
   install candidate on 26.04 and were enough to build. An unknown release
   gets the common list and a note on stderr.
6. **Docs.** README Prerequisites and `.agents/README.md`.

### Verification

- Ubuntu 26.04: `scripts/build.sh` exits 0 (every target, `GeoZui4D`
  included); `cd build && ctest --output-on-failure`: 1 of 1 passed
  (`test_libgz4d`).
- `install_deps.sh`: `--list`, `--dry-run` and `--help` run here; the
  24.04, an unknown Ubuntu and a non-Debian case run against a substituted
  os-release file; an add and a drop checked with a temporary table entry.
- **Not verified: Ubuntu 24.04.** No container tool (docker, podman) is on
  this machine, so the 24.04 half of the acceptance has not been run. The
  changes were written to be neutral there (`OPTIONAL_COMPONENTS` needs
  CMake 3.11; a non-const to const pointer conversion and a `const_cast`
  to the same type compile with the older GDAL; GCC 13 accepts the warning
  flag), but that is reasoning, not a build. Whether the Python bindings
  were being built on 24.04 before this change was not checked: if Python 2
  is absent there they are now skipped, as on 26.04.
- The application was built, not started.

### Gaps

- The Togl warnings remain (two, in the vendored file).
- The Python bindings are dead code on both releases unless Python 2 is
  installed; whether to port or remove them is not decided here.

## Trial counts

Light-level trial (no review loop), piece of work 1 (the owner counted it, 2026-11-24).

- **Start**: 2026-11-24 11:24 -05:00 (worktree created).
- **Merge**: not merged yet.
- **Defects found later**: none recorded yet.
- **Redone because the first result was not what was meant**: none
  recorded yet.
