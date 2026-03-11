# ADR-0001: Upgrade to CMake 3.14 and C++17

## Status

Accepted

## Context

The build system used CMake without a minimum version requirement and compiled
with `-std=gnu++0x` (a deprecated alias for C++11 with GNU extensions). This
prevented use of modern C++ features like `std::filesystem`, `std::optional`,
structured bindings, and `if constexpr`. The GNU extensions flag also reduced
portability.

A top-level CMakeLists.txt was added to enable unified builds of both libgz4d
and GeoZui4D from the repository root, replacing the previous workflow of
building each sub-project separately.

## Decision

Upgrade to CMake 3.14 minimum and C++17 standard.

- Set `CMAKE_CXX_STANDARD 17` and `CMAKE_CXX_EXTENSIONS OFF` in the
  top-level CMakeLists.txt
- Remove all `-std=gnu++0x` flags from sub-project CMakeLists.txt files
- Sub-projects inherit the C++ standard from the top-level configuration

CMake 3.14 was chosen as the minimum because it is the oldest version that
fully supports `CMAKE_CXX_STANDARD 17` and is available in Ubuntu 20.04+.

## Consequences

**Positive:**
- Modern C++ features are available throughout the codebase
- `std::filesystem` can replace Boost.Filesystem (see ADR-0002)
- Portable builds -- no GNU extensions dependency
- Unified build from repository root simplifies the developer workflow

**Negative:**
- Drops support for compilers that lack C++17 (GCC < 7, Clang < 5)
- Drops support for CMake versions older than 3.14
