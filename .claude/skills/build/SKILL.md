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

Build the project from the repository root with the project's tracked build
script, `scripts/build.sh`, then run the test suite. Use after code changes
to verify that compilation and the tests succeed.

`scripts/build.sh` configures `build/` with `-DBUILD_GTESTS=ON` (GoogleTest
must be installed, `libgtest-dev` on Ubuntu) and builds every target, with
one job per core.

## Steps

### Default: Build and test

```bash
scripts/build.sh
(cd build && ctest --output-on-failure)
```

Report:
- Build success or failure
- Warning count (if any)
- Error details (if build fails)
- Test results (passed, failed)

### With `--install`

```bash
scripts/build.sh
(cd build && ctest --output-on-failure)
cmake --install build --prefix install
```

Report:
- Build, test and install status
- Install location: `install/bin/GeoZui4D`

## Guidelines

- Always build from the repository root using the top-level CMakeLists.txt
- Run the full build, with the tests, as part of any audit: this project's
  rule
- To limit the number of parallel jobs (for example on a machine short of
  memory), run `scripts/build.sh --parallel <N>`: the script passes its
  arguments to the build step, and the last `--parallel` wins. Pick N for
  the machine, from its cores and free memory.
- Report warnings even on successful builds -- they may indicate issues
- If the build fails, read the error output and suggest fixes
- Do not modify CMakeLists.txt to fix build errors without asking first
