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

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:25 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (both fixes are in the issue's own Proposed section; no scope change)
**After**: issue-actions
**Decision**: proceed

The Actions ask to prefer removing the unused stub over implementing a percent-encoder, and to confirm the warning is gone in a fresh build log. The issue body offers both ("Either implement it ... or remove the unused declaration"), and `git grep` finds no C++ caller, so the choice does not change what the work is for. The plan should take removal unless it finds a caller (Tcl, Python bindings, or another consumer of the installed header); the owner may veto in favour of implementing.

## Plan Authored
**Status**: complete
**When**: 2026-11-15 09:26 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-13/plan.md` at `3c889f0`

Remove the unused empty `URI::encode` stub from URI.h. Searched C++, Tcl, Python, SWIG and CMake files: no caller. Verify with a fresh default build log showing no -Wreturn-type for URI.h, and run test_libgz4d.

## Plan Review
**Status**: complete
**When**: 2026-11-15 09:29 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: ready

**Issue**: #13 — URI::encode in URI.h has an empty body and returns nothing (-Wreturn-type)
**Plan**: `.agent/work-plans/issue-13/plan.md` at `3c889f0`
**Branch**: `feature/issue-13`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | One 3-line deletion in one header; single small PR. |
| Issue alignment | Good | Takes the issue's "remove the unused declaration" option, as the Issue Review and the proceed checkpoint preferred; keeps the implement fallback if a caller turns up. Serves (as checked): "None (hygiene on the build; the project's `docs/roadmap.md` Direction lines do not cover it); design document: none." Breaks no stated project rule. |
| File targeting | Good | `URI.h:274-276` verified empty; `git grep` confirms no caller of `encode` in C++, Tcl, SWIG `.i` or the `boost.python` bindings (no `URI` reference there). |
| Consequences | Needs work | The includer list (SocketListener, SocketReceiver, IPSocket, gtest/URI.cpp) is incomplete; see finding 1. No caller exists, so the outcome is unchanged. |
| Principle alignment | Good | Only what's needed: removal over an unused encoder. Test what breaks: no new test is planned; for a deletion the evidence is the build log and the existing `test_libgz4d` run, which the plan should say explicitly (finding 3). Design picture: no change, no design document, stated. Prior art: stated. |
| ADR compliance | Good | No ADR triggered (no build config, paths or new files). |
| ROS conventions | N/A | Not a ROS project. |

### Findings

1. **[Consequences]** — The table lists four includers, but `URI.h` reaches more: `GeoZui4D/main/engine/Object.h`, `libgz4d/cxx/boost.python/socket/IPAddress.cpp` and `IPSocket.cpp`, `socket/TCPSocket.h`, `socket/UDPSocket.h`, `sonar/Simrad/net/Server.h`, and `test/cat_ais.cpp`, `cat_ais8.cpp`, `show_ais.cpp`, `list_ipAddresses.cpp`, `udp.cpp`. None calls `encode`, so this is a record-accuracy fix, not a blocker; correct the list in the PR description. (suggestion)
2. **[Issue alignment]** — Steps 3 and 4 do not fit together: `BUILD_GTESTS` defaults to OFF (`libgz4d/cxx/CMakeLists.txt:449`), so a "fresh build with default options" does not compile `gtest/URI.cpp` and does not build `test_libgz4d`. Do the acceptance check with a default-options build, then build with `-DBUILD_GTESTS=ON` (as `scripts/build.sh` does) and run ctest. Grep both logs for `-Wreturn-type`/`URI.h`. (suggestion)
3. **[Principle alignment]** — Say in the PR that no new test is added because a deletion has nothing to exercise, and that the before/after build-log grep is the check that fails without the fix. (suggestion)

### Summary

A minimal, well-supported plan: the stub is verified empty and verified uncalled, and removal matches the issue and the checkpoint decision. The three suggestions tighten the verification record and none of them blocks implementation.

### Recommended Actions

- [ ] List every includer of `URI.h` in the PR's consequence record (finding 1).
- [ ] Verify with both a default-options build (acceptance) and a `-DBUILD_GTESTS=ON` build plus ctest; grep both logs (finding 2).
- [ ] State in the PR why no new test is added and that the build-log grep is the regression check (finding 3).

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:30 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (plan review verdict ready, no must-fix; suggestions folded into the implement handoff)
**After**: plan
**Decision**: proceed

The Plan Review at 70c73bb found no must-fix. Its three suggestions go to the implement pass: list every includer of URI.h in the PR; verify with a default-options build (the acceptance check) and a -DBUILD_GTESTS=ON build with ctest, grepping both logs for -Wreturn-type and URI.h; and say in the PR why no new test is added.

## Implementation
**Status**: complete
**When**: 2026-11-15 10:04 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Branch**: feature/issue-13 at `06486ef`

- `libgz4d/cxx/include/gz4d/URI.h`: removed the empty `static std::string encode(std::string const&)` member (and its preceding blank line) from `struct URI` (commit 06486ef).
- Includers of URI.h (direct): `ais/SocketListener.h`, `socket/IPSocket.h`, `socket/SocketReceiver.h`, `gtest/URI.cpp`. Through those: `GeoZui4D/main/engine/Object.h` (and everything in the app that includes it), `boost.python/socket/IPAddress.cpp`, `boost.python/socket/IPSocket.cpp`, `socket/TCPSocket.h`, `socket/UDPSocket.h`, `sonar/Simrad/net/Server.h`, `gtest/socket.cpp` (not in the plan review's list), and `test/cat_ais.cpp`, `cat_ais8.cpp`, `show_ais.cpp`, `list_ipAddresses.cpp`, `udp.cpp`. `git grep encode` over `*.h/*.cpp/*.hpp/*.tcl/*.i` finds no call to `URI::encode`; both full builds below compile every built includer with the member gone.
- No new test: a deletion leaves nothing to exercise. The regression check is the build-log grep: before the change the default build logs 166 `-Wreturn-type` warnings at `URI.h:276`; after it, none.

Verification (fresh build dirs in the session scratchpad, `--parallel 6`):

| Build | `-Wreturn-type` lines | `URI.h` lines | Result |
|---|---|---|---|
| Default options, before (branch base = main code) | 224 (166 at URI.h:276) | 332 | exit 0 |
| Default options, after | 58 | 0 | exit 0 |
| `-DBUILD_GTESTS=ON`, after | 58 | 0 | exit 0; ctest: test_libgz4d passed (1/1) |

The remaining 58 `-Wreturn-type` warnings are in other headers (Box.h:133, dtag/Chip.h, dtag/DTAG.h, sonar/RDI/Sample.h, Simrad/net/Parameter.h and Response.h, v4l/Stream.h), outside this issue's scope.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-15 10:05 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: approved

**Branch**: feature/issue-13 at `ffca29b`
**Base**: main
**Depth**: Deep (reason: 203 changed lines on the whole branch diff, over the 200-line Deep threshold; the code change itself is 4 deleted lines in URI.h, the rest is plan.md/progress.md)
**Must-fix**: 0 | **Suggestions**: 0
**Round**: 1 | **Ship**: recommended — no must-fix findings

Specialists: static analysis (cppcheck not installed; `g++ -std=c++17 -Wall -Wextra -fsyntax-only` on a URI.h includer is clean), governance (no ADR triggered; no vendored or IVS file touched), plan drift (diff matches the plan's single file and approach), Claude adversarial (no findings), cross-model Gemini (no findings) and Codex (no findings).

### Findings
- [ ] No issues found. LGTM.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 10:06 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (pre-push review round 1 approved, ship recommended, no findings)
**After**: publish
**Decision**: publish

The Local Review (Pre-Push) at 008c0b2 approved with 0 must-fix and 0 suggestions; Gemini and Codex ran and found nothing. Publishing: push and open the pull request with the review's Decision summary.

## Integrated Review
**Status**: complete
**When**: 2026-11-15 10:18 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**PR**: #21 at `fc317c8`
**Sources**: 1 (Local Review (Pre-Push) @ `ffca29b` (covers `fc317c8` via bookkeeping), approved, no findings; Claude adversarial, Gemini and Codex ran clean). GitHub: 0 reviews, 0 inline comments, 0 conversation comments, no Copilot review check-run.
**Cross-source confirmations**: 0
**CI**: all-pass

CI note: the repository has no CI workflow (issue 12 will add one), so the PR head has 0 check-runs; `all-pass` is recorded vacuously because nothing failed and nothing is pending. No build or test was verified by CI. Triage confirmed locally that the diff removes only the empty `URI::encode` stub from `libgz4d/cxx/include/gz4d/URI.h` and that no caller of it exists in the tree.

### Findings
None.

### False positives
None.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 11:07 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: owner
**After**: merge
**Decision**: merge

Asked "Merge PR #21? (It closes #13; the worktree and branch are removed after.)", with the note that the repository has no CI and no Copilot review, so GitHub ran no checks, the owner answered: "Merge".
