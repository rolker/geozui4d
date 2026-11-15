# Plan: URI::encode in URI.h has an empty body and returns nothing (-Wreturn-type)

## Issue

https://github.com/rolker/geozui4d/issues/13

## Context

`libgz4d/cxx/include/gz4d/URI.h:274-276` declares `static std::string encode(std::string const&)` with an empty body: undefined behaviour if called, and GCC emits `-Wreturn-type` in every translation unit including the header (`ais/SocketListener.h`, `socket/SocketReceiver.h`, `socket/IPSocket.h`, `gtest/URI.cpp`). The issue-actions checkpoint was answered proceed, preferring removal.

## Approach

1. **Search for callers first** — searched all of `*.h/*.hpp/*.cpp/*.tcl/*.py/*.i/*.txt/*.cmake` in the repo for `URI::encode` and `encode(`. Only hits are the unrelated Python AIS encoders (`GeoZui4D/main/gzmod/ais/`) and `.encode('ascii')` string calls in `libgz4d/cxx/applications/*.py`; no Tcl, SWIG/Python binding or other consumer references the header's `encode`. Nothing is installed from outside this tree. If a caller turns up during implementation, switch to implementing a percent-encoder with a gtest case.
2. **Remove the stub** — delete the 3-line `encode` member (and the blank line before it) from `struct URI` in `URI.h`.
3. **Fresh build and log check** — configure a fresh build dir with default options, build with output to a log, and grep the log for `-Wreturn-type` / `URI.h`: expect none (record the pre-change baseline from the same command on `main` if cheap).
4. **Run tests** — `test_libgz4d` (includes `gtest/URI.cpp`) must still compile and pass.

## Files to Change

| File | Change |
|------|--------|
| `libgz4d/cxx/include/gz4d/URI.h` | Remove the unused empty `URI::encode` |

## Prior Art

Nothing new is built. The header has a parser (query splitting) but no decoder or percent-handling to mirror, so a new encoder would be unused API surface.

## Serves

None (hygiene on the build; the project's `docs/roadmap.md` Direction lines do not cover it); design document: none.

## Design Picture

No change to the design picture; the project has no design document.

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Only what's needed | Remove the dead stub rather than add an unused encoder |
| Verify before documenting | Caller search and build log are the evidence, recorded in the PR |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| ADR-0001 | No | No build config change |
| ADR-0002 | No | Not path-related |
| ADR-0003 | No | No new files |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| `URI.h` public struct | Includers (SocketListener, SocketReceiver, IPSocket, gtest/URI.cpp) | Yes: none call `encode`; full build verifies |

## Open Questions

- None; owner may veto removal in favour of implementing.

## Estimated Scope

Single small PR.
