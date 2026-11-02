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
# stops (issue #9). Turning it on is not enough on its own for the registered
# test command, `ctest --test-dir build --output-on-failure`: enable_testing()
# is called only in libgz4d/cxx/CMakeLists.txt, not in the top-level
# CMakeLists.txt, so build/ gets no CTestTestfile.cmake and ctest there finds
# no tests and exits 0 whatever BUILD_GTESTS says. test_libgz4d is reachable
# only from build/libgz4d/cxx and below. Both fixes, the gtest header and a
# top-level enable_testing(), belong to #9; after them, add -DBUILD_GTESTS=ON
# here.
# Note that #9 also means a fresh clone or worktree does not configure at all
# yet; the main checkout does.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

cmake -S . -B build
cmake --build build --parallel "$(nproc 2>/dev/null || echo 4)" "$@"
