---
issue: 13
---

# Issue #13 — URI::encode in URI.h has an empty body and returns nothing (-Wreturn-type)

## Issue Review
**Status**: complete
**When**: 2026-11-15 09:25 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #13

### Scope Assessment

**Well-scoped?** Yes - one empty function in one header, two acceptance checks, fits a single small PR.
**Right repo?** Yes - the defect is in `libgz4d/cxx/include/gz4d/URI.h` in this repo.
**Serves**: none (the project's `docs/roadmap.md` Direction lines do not cover warning cleanup; it is hygiene on the build the acceptance case depends on) - `docs/design.md`: none, the project has no design document.
**Design picture?** No change - no part of how the pieces fit changes. The project has no design document.
**Dependencies**: none identified. #9 (gtest suite builds and runs) is closed, so a test can be added.

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Only what's needed | Watch | Verified `git grep`: no C++ caller of `URI::encode`; the header has no decoder or percent-handling to mirror. Removing the unused stub is the smaller change; implementing adds untested-by-use API surface. |
| Verify before documenting | OK | The issue's claims match the source: the body at URI.h:274-276 is empty. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| ADR-0001 (CMake 3.14 / C++17) | No | No build config change. |
| ADR-0002 (std::filesystem) | No | Not path-related. |
| ADR-0003 (Apache 2.0) | No | No new files needing headers unless a test file is added; the existing `gtest/URI.cpp` is reused. |

### Consequences

- If removed: no other file references it; check `libgz4d/cxx/gtest/URI.cpp` still compiles.
- If implemented: add a `URI::encode` case to `libgz4d/cxx/gtest/URI.cpp`.
- Verify with a fresh default build that `-Wreturn-type` no longer fires for `URI.h`, per the Post-Task Verification rule.

### Recommendations

- Prefer removing the unused `URI::encode` stub; implement it only if the owner wants a percent-encoder, with a test in `libgz4d/cxx/gtest/URI.cpp`.
- Confirm in the fresh build log that no `-Wreturn-type` warning remains for `URI.h` (SocketListener.h and the URI gtest include it).

### Actions
- [ ] Prefer removing the unused `URI::encode` stub; implement it only if the owner wants a percent-encoder, with a test in `libgz4d/cxx/gtest/URI.cpp`.
- [ ] Confirm in the fresh build log that no `-Wreturn-type` warning remains for `URI.h` (SocketListener.h and the URI gtest include it).
