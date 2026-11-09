---
name: build
description: Build and optionally install GeoZui4D. Run after code changes to verify compilation.
---

# Build

## Usage

```
/build
/build --install
```

## Overview

Build the project from the repository root. Use after code changes to verify
compilation succeeds.

## Steps

### Default: Build

```bash
mkdir -p build && cd build && cmake .. && make -j$(nproc)
```

Report:
- Build success or failure
- Warning count (if any)
- Error details (if build fails)

### With `--install`

```bash
mkdir -p build && cd build && cmake .. && make -j$(nproc)
cmake --install . --prefix ../install
```

Report:
- Build and install status
- Install location: `install/bin/GeoZui4D`

## Guidelines

- Always build from the repository root using the top-level CMakeLists.txt
- Report warnings even on successful builds -- they may indicate issues
- If the build fails, read the error output and suggest fixes
- Do not modify CMakeLists.txt to fix build errors without asking first
