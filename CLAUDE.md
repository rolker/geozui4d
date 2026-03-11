# Claude Code -- GeoZui4D

@AGENTS.md

## Project Overview

GeoZui4D is a 4-dimensional geographic visualization system for ocean mapping,
marine navigation, and underwater exploration. Developed at the Data
Visualization Research Lab (VisLab), Center for Coastal and Ocean Mapping
(CCOM), University of New Hampshire.

**Tech stack**: C++17, Tcl/Tk (GUI), OpenGL (rendering), CMake (build system)

**Architecture**: Two main components:
- `libgz4d/` -- Core library (geo, sonar, X3D, coordinate systems, DSP, etc.)
- `GeoZui4D/` -- Application (rendering engine, objects, interactors, Tcl/Tk GUI)

## Build Commands

```bash
# Build
mkdir -p build && cd build && cmake .. && make -j$(nproc)

# Install
cd build && cmake --install . --prefix ../install

# Run
./install/bin/GeoZui4D
```

## Key Directories

| Path | Contents |
|------|----------|
| `libgz4d/cxx/include/gz4d/` | Library headers (40+ subdirectories) |
| `libgz4d/cxx/src/` | Library source (X3D parser) |
| `GeoZui4D/main/engine/` | Rendering engine, scene management |
| `GeoZui4D/main/objects/` | Object types (bathymetry, vessels, sonar, charts) |
| `GeoZui4D/main/interactors/` | Input handling (mouse, spaceball, wiimote) |
| `GeoZui4D/main/*.tcl` | Tcl/Tk GUI scripts |
| `GeoZui4D/main/scripts/` | Tcl extension scripts |
| `docs/decisions/` | Architecture Decision Records |

## File Patterns

- Headers: `.h` (1400+), `.hpp` (8)
- Source: `.cpp` (900+), `.cxx` (3)
- GUI scripts: `.tcl` (240)
- Scene files: `.gzx` (XML-based)

## Coding Conventions

- **C++ standard**: C++17 (`CMAKE_CXX_STANDARD 17`, no GNU extensions)
- **Filesystem**: Use `std::filesystem`, not `boost::filesystem` (ADR-0002)
- **Headers**: `.h` for C++ headers (project convention)
- **Namespaces**: `gz4d::` for library code
- **Build**: CMake 3.14+, build from repo root

## Third-Party Vendored Code -- DO NOT MODIFY

These directories contain vendored third-party code. Do not modify them:

- `GeoZui4D/main/Togl/` -- OpenGL widget for Tcl/Tk
- `GeoZui4D/main/Togl2.0/` -- Newer Togl version
- `GeoZui4D/main/BWidget-1.7.0/` -- Tcl/Tk widget set
- `libgz4d/cxx/include/gz4d/util/rapidxml/` -- Header-only XML parser

## IVS Copyright Files -- DO NOT MODIFY

Files in `GeoZui4D/main/widgets/` with Interactive Visualization Systems Inc.
copyright (Sv_Matrix4, Sv_Quaternion, Sv_Raster) predate this project. Do not
modify them without explicit approval.

## Architecture Decision Records

See [`docs/decisions/`](docs/decisions/) for recorded decisions:
- ADR-0001: CMake 3.14 and C++17
- ADR-0002: std::filesystem over boost::filesystem
- ADR-0003: Apache 2.0 license

## Skills

| Skill | Purpose |
|-------|---------|
| `/plan-task` | Generate a work plan for an issue |
| `/review-pr` | Review a PR against project conventions |
| `/audit-project` | Audit project for documentation and build health |
| `/research` | Research a topic, maintain digest |
| `/brand-guidelines` | CCOM/JHC brand identity and editorial standards |
| `/build` | Build and install the project |
