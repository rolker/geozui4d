#!/usr/bin/env bash
# scripts/build.sh -- the project's build command, as the agent workspace
# adapter runs it (its settings for this project say build=./scripts/build.sh;
# the adapter runs a stored command without a shell, so the two cmake steps
# live here). Also usable by hand from anywhere: it works from the repository
# root, which it finds from its own location.
#
# Configures build/ with the libgz4d gtest suite enabled (BUILD_GTESTS=ON, off
# by default in libgz4d/cxx/CMakeLists.txt) so that the test command,
# `ctest --test-dir build --output-on-failure`, finds the tests; then builds
# every target. Extra arguments are passed to the build step, e.g.
# `scripts/build.sh --target GeoZui4D`.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

cmake -S . -B build -DBUILD_GTESTS=ON
cmake --build build --parallel "$(nproc 2>/dev/null || echo 4)" "$@"
