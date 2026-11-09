#!/usr/bin/env bash
# scripts/build.sh -- the project's build command, as the agent workspace
# adapter runs it (its settings for this project say build=./scripts/build.sh;
# the adapter runs a stored command without a shell, so the two cmake steps
# live here). Also usable by hand from anywhere: it works from the repository
# root, which it finds from its own location.
#
# Configures build/, then builds every target. Extra arguments are passed to
# the build step, e.g. `scripts/build.sh --target GeoZui4D`.
#
# The libgz4d gtest suite is on (-DBUILD_GTESTS=ON), so GoogleTest must be
# installed (libgtest-dev on Ubuntu); configure stops if it is missing. Run
# the tests afterwards with `cd build && ctest --output-on-failure`
# (`ctest --test-dir` needs CMake 3.20, above the project's 3.14 minimum).
# The build uses one job per core (4 if nproc is missing).
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

cmake -S . -B build -DBUILD_GTESTS=ON
cmake --build build --parallel "$(nproc 2>/dev/null || echo 4)" "$@"
