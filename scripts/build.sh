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
# The libgz4d gtest suite stays off: with BUILD_GTESTS=ON the configure step
# looks for a header named gtest/test.h, which GoogleTest does not ship, and
# stops (issue #9). Once #9 lands, add -DBUILD_GTESTS=ON to the configure
# line so that `ctest --test-dir build --output-on-failure`, the registered
# test command, finds test_libgz4d. Until then it finds no tests and exits 0.
# Note that #9 also means a fresh clone or worktree does not configure at all
# yet; the main checkout does.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

cmake -S . -B build
cmake --build build --parallel "$(nproc 2>/dev/null || echo 4)" "$@"
