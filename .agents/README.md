# Agent Guide: gz4d (GeoZui4D)

> A 4-dimensional (3D space plus time) geographic visualization system for
> ocean mapping, marine navigation and underwater exploration, from the Data
> Visualization Research Lab at CCOM, University of New Hampshire.

This guide is for agents working in this repository. The rules agents must
follow are in [`AGENTS.md`](../AGENTS.md); the Claude Code adapter is
[`CLAUDE.md`](../CLAUDE.md). This file says what the code is and where to
look.

GeoZui4D can be used with the
[agent workspace](https://github.com/rolker/agent_workspace), a separate
project for developing software with AI agents, but does not depend on it;
this guide stands on its own. See that repository for what it does and how
to set a project up with it. The adapter commands under Build & Test apply
once that is done.

Every claim here was checked against the source on 2026-11-02; when the
code and this guide disagree, the code wins, and this guide is wrong.

## Package Inventory

| Component | Language | Description |
|-----------|----------|-------------|
| `libgz4d/cxx` | C++17 | The support library. 392 headers under `include/gz4d/`, almost all header-only; one compiled library, `gz4d_x3d` (the X3D parser in `src/x3d/`); 114 stand-alone test and utility programs in `test/`, each built only when its dependencies are found; a gtest suite in `gtest/` behind `BUILD_GTESTS`. |
| `GeoZui4D/main` | C++17, Tcl/Tk | The application: one executable, `GeoZui4D`, built from `main.cpp`, `MainWindow.cpp`, `SubWindow.cpp` and the sources that `engine/`, `interactors/`, `objects/`, `utilities/` and `widgets/` add to `GEOZUI4D_SRC` (their `add_library` lines are commented out). The GUI is Tcl/Tk scripts that the executable runs. |
| `libgz4d/python` | Python 2 era | Utilities (`iso8211`, `s57`, `Wiimote`, `pygtk`, `wxPython`). Not built by CMake. |

## Repository Layout

```
gz4d/
├── CMakeLists.txt            # Top level: builds libgz4d/cxx, then GeoZui4D/main
├── scripts/build.sh          # Configure + build, as the workspace adapter runs it
├── libgz4d/
│   ├── cxx/
│   │   ├── include/gz4d/     # Library headers (header-only modules; see below)
│   │   ├── src/x3d/          # The gz4d_x3d library: X3D node classes
│   │   ├── test/             # 114 small programs, one .cpp each (tools, not unit tests)
│   │   ├── gtest/            # Unit tests: one executable, test_libgz4d
│   │   ├── boost.python/     # Python bindings (option GZ4D_PYTHON, default ON)
│   │   └── CMakeLists.txt    # Finds every optional dependency; options listed below
│   └── python/               # Python utilities, not part of the CMake build
├── GeoZui4D/main/
│   ├── main.cpp              # Entry point
│   ├── MainWindow.cpp        # Tcl interpreter setup; the four GUI modes' start scripts
│   ├── engine/               # Scene3D, rendering, scene management, gzCommandInterface
│   ├── interactors/          # Input handling: picking, zooming, rubberband, tools, animators
│   ├── objects/              # Scene object types: dxf, GeoData, MissionPlan, nodeTypes, ogr, S57, Sonar
│   ├── utilities/            # Helper sources added to the executable
│   ├── widgets/              # LightDirector and the Sv_* IVS-copyright files (do not modify)
│   ├── Togl/                 # VENDORED: Tcl/Tk OpenGL widget, built as gz_togl (do not modify)
│   ├── Togl2.0/              # VENDORED: newer Togl; present but not referenced by CMake (do not modify)
│   ├── BWidget-1.7.0/        # VENDORED: Tcl/Tk widget set, installed as scripts (do not modify)
│   ├── *.tcl                 # 15 Tcl/Tk GUI scripts (visualizer.tcl is the start script)
│   ├── scripts/              # Tcl extension scripts, installed to lib/GeoZui4D
│   ├── gzmod/                # Python extension modules, installed to lib/GeoZui4D/gzmod
│   └── bitmaps/              # Images installed with the app
└── docs/
    ├── decisions/            # ADRs 0001-0003 and their index
    ├── roadmap.md            # Direction section read by the workspace sit-rep
    └── work-plans/           # Not tracked until a plan lands; the project's own /plan-task skill writes plans here
```

## Architecture Overview

**libgz4d** is a collection of toolkit-independent C++ modules that assume
only C++ and OpenGL (`include/gz4d/gz4d.h`, the Doxygen main page). Every
entity is in namespace `gz4d`. The header subdirectories are the modules:
`geo` and `crs` (coordinates), `sonar`, `acoustic`, `ais`, `nav`, `s57`,
`netcdf`, `gdal`, `ogr`, `image`, `av` (audio/video), `dsp`, `glo` (OpenGL
objects), `render`, `scene`, `x3d`, device interfaces (`IMU`, `serial`,
`socket`, `Spaceball.h`, `Wiimote.h`, `Kinect.h`, `v4l`, `midi`), and
`util`, which holds the vendored RapidXML parser. Only the X3D node classes
are compiled, into `gz4d_x3d`; everything else is included.

**GeoZui4D** links `gz4d_x3d`, the vendored `gz_togl`, Tcl/Tk stubs, GDAL,
Boost (system, program_options, iostreams, thread), OpenGL, GLUT, X11,
spnav (the spaceball library, required on non-cross builds) and the image
libraries CMake finds. `main()` (`GeoZui4D/main/main.cpp`) initialises
GLUT, builds a `Scene3D`, wires up a dotted naming hierarchy of
"neighborhoods" (posables, animators, couplings, interactors, events),
creates the `MainWindow` and a `gzCommandInterface` that exposes the scene
to Tcl, then calls `MainWindow::Init`, which starts a Tcl interpreter with
one of four start scripts, chosen by the command line (`MainWindow.cpp`,
`parseCommandLine`): the default, `-nav`, `-cp` and `-wall`, which set
`guiMode` to `zui`, `nav`, `cp` and `wall`. Each script then sources
every `GeoZui4D.conf` it finds, in this order: the directory named by the
`GEOZUI4D` environment variable, `~/.GeoZui4D/`, the working directory.
These are three independent checks, not a first-found lookup, so a value
set in a later file overrides the same value from an earlier one. A config
file may set `appdir`, `datadir` and the other variables the script
declares. Then it sources `visualizer.tcl` from
`appdir`. The default, `-cp` and `-wall` scripts set `appdir` to
`../lib/GeoZui4D` relative to the executable, so those modes only work
installed: the Tcl scripts, bitmaps and BWidget are found relative to the
binary. The `-nav` script sets `appdir` to an empty string, so GeoNav3D
looks for `visualizer.tcl` in the working directory unless `GeoZui4D.conf`
sets `appdir`.

The top-level `CMakeLists.txt` builds `libgz4d/cxx` first, points
`GeoZui4D/main` at the in-tree headers and the `gz4d_x3d` target, and
forces the AIS module off (`ais_INCLUDE_DIR` and `ais_LIBRARY` empty) because
the libais API has diverged.

## Key Files to Read First

1. `CMakeLists.txt` (top level) -- how the two components are wired together.
2. `libgz4d/cxx/CMakeLists.txt` -- every optional dependency and the options
   `GZ4D_PYTHON` (default ON), `GZ4D_OPENCV` (default OFF), `BUILD_GTESTS`
   (default OFF), `GTESTS_NEED_PTHREADS` (default OFF).
3. `libgz4d/cxx/include/gz4d/gz4d.h` -- the library's module map (Doxygen groups).
4. `GeoZui4D/main/main.cpp` and `GeoZui4D/main/MainWindow.cpp` -- startup and
   the Tcl start scripts for the four GUI modes.
5. `GeoZui4D/main/engine/Scene3D.h` and `engine/gzCommandInterface.h` -- the
   scene and its Tcl command surface.
6. `docs/decisions/` -- ADR-0001 (CMake 3.14, C++17), ADR-0002
   (`std::filesystem`, not `boost::filesystem`), ADR-0003 (Apache 2.0).

## Build & Test

From the agent workspace, through its adapter (`gz4d` is the name used for
this checkout; substitute yours):

```bash
WS_ROOT="$(cat ~/.claude/agent-workspace-root)"
"$WS_ROOT/.agent/scripts/adapter" --project gz4d build   # runs ./scripts/build.sh
"$WS_ROOT/.agent/scripts/adapter" --project gz4d test    # ctest --test-dir build --output-on-failure
```

By hand:

```bash
scripts/build.sh                      # cmake -S . -B build; cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix install
install/bin/GeoZui4D [files...]
```

What to know:

- The gtest suite is one executable, `test_libgz4d`, from nine sources in
  `libgz4d/cxx/gtest/` (angle, box, color, geocam, matrix, socket, time,
  timeSequence, URI). It exists only when the build was configured with
  `BUILD_GTESTS=ON`, which cannot configure today (the CMake looks for
  `gtest/test.h`; issue #9), so `scripts/build.sh` leaves it off.
- The test command finds zero tests and exits 0, and would do so even with
  the suite built: `enable_testing()` is called only in
  `libgz4d/cxx/CMakeLists.txt`, not in the top-level `CMakeLists.txt`, so
  `build/` has no `CTestTestfile.cmake`. A built `test_libgz4d` is
  reachable only with `ctest --test-dir build/libgz4d/cxx`. A top-level
  `enable_testing()` belongs to issue #9 with the gtest header fix.
- **A fresh clone or worktree does not configure** (issue #9): nineteen
  generated `CMakeLists.txt` files reference an in-source `CMakeFiles/`
  directory that exists only as an ignored leftover in the owner's main
  checkout, and the version lookup needs a Subversion working copy. Build in
  the main checkout until #9 lands.
- `libgz4d/cxx/test/` is not a test suite: it is 114 utility programs
  (converters, players, calibration tools), each skipped with a message
  when its libraries are missing.
- Ubuntu package prerequisites are listed in the README under Building.
- The install step is required to run the app (see Architecture Overview).
- The build directory is `build/` at the repository root, which `.gitignore`
  ignores with the rule `build/`. That rule also matches
  `.claude/skills/build/`, so that skill is not under version control
  (issue #7 tracks the fix).

## Dependencies

| Component | Depends On | What It Uses |
|-----------|-----------|--------------|
| `GeoZui4D/main` | `libgz4d/cxx` | Headers under `include/gz4d/` and the `gz4d_x3d` library (`find_path(gz4d_INCLUDE_DIR gz4d/gz4d.h)` and `find_library` for `gz4d_x3d`; the top-level build sets both) |
| `GeoZui4D/main` | `Togl/` (vendored) | `gz_togl` static library, the Tk OpenGL widget |
| `libgz4d/cxx` | Boost, GDAL, optional image/audio/video/NetCDF/Bluetooth libraries | Found in `libgz4d/cxx/CMakeLists.txt`; a missing optional library skips the programs that need it |

## Common Pitfalls

- **Do not modify vendored code**: `GeoZui4D/main/Togl/`, `Togl2.0/`,
  `BWidget-1.7.0/`, `libgz4d/cxx/include/gz4d/util/rapidxml/`.
- **Never modify the IVS-copyright files** in `GeoZui4D/main/widgets/`
  (`Sv_Matrix4`, `Sv_Quaternion`, `Sv_Raster`); `AGENTS.md` lists this under
  Never.
- **`std::filesystem`, never `boost::filesystem`** (ADR-0002). Boost
  Filesystem is no longer linked; a stray use fails at link time.
- **Headers are `.h`**, even for C++ (four `.hpp` files exist; new headers use `.h`).
- **Changes to a `CMakeLists.txt`** get a build-impact assessment in review
  (the project's `/review-pr` skill, step 5; issue #7 ports the rule to `AGENTS.md`).
- **Sub-project `CMakeLists.txt` files were generated** (`libgz4d/cxx/src/`
  and `test/` say "auto generated by gen_cmake.py"); edit by hand with care.
- **The version comes from Subversion.** Both `GeoZui4D/main/CMakeLists.txt`
  and `libgz4d/cxx/CMakeLists.txt` call `Subversion_WC_INFO` when `svn` is
  installed and set `SUBVERSION_REVISION` (used as the CPack patch
  version). Only `GeoZui4D/main` builds a version string from it
  (`GZ4D_VERSION`, `4.0.<revision>`), with an `unknown` fallback that
  applies only when `svn` is not installed; `libgz4d/cxx` has no fallback.
  With `svn` installed and no Subversion working copy, as in any git clone
  or worktree, `Subversion_WC_INFO` fails in either file and configure
  stops: this is point 2 of issue #9, not an expected missing version.
- **The project's own skills** (`.claude/skills/`) include four that
  duplicate agent-workspace skills by name; in a workspace session the bare
  `/plan-task`, `/research`, `/audit-project` and `/brand-guidelines` are
  the project's copies and `/agent-workspace:<skill>` the workspace's.
  Issue #7 tracks removing the duplicates.
