# ADR-0002: Replace boost::filesystem with std::filesystem

## Status

Accepted

## Context

The codebase used `boost::filesystem` extensively across 85 files for path
manipulation, file existence checks, directory iteration, and related
operations. With the upgrade to C++17 (ADR-0001), `std::filesystem` became
available as a standard library alternative, eliminating the need for this
Boost dependency.

## Decision

Replace all uses of `boost::filesystem` with `std::filesystem` across the
entire codebase.

- Change `#include <boost/filesystem.hpp>` to `#include <filesystem>`
- Replace `boost::filesystem::` with `std::filesystem::` (or namespace alias `fs::`)
- Remove Boost.Filesystem from CMake link dependencies where it was the
  only Boost component used
- Retain other Boost dependencies (e.g., Boost.Python) where still needed

## Consequences

**Positive:**
- Removes a compile-time and link-time dependency on Boost.Filesystem
- Reduces the Boost surface area, moving toward standard-library-only code
  where possible
- `std::filesystem` is part of the C++ standard -- no external library needed

**Negative:**
- `std::filesystem` behavior differs slightly from `boost::filesystem` in
  edge cases (e.g., error handling, symlink resolution) -- these were reviewed
  during the migration and no issues were found
- Requires C++17-capable compiler (already mandated by ADR-0001)
