# Plan: Agent-workspace readiness: remove duplicated skills, port gz4d conventions, track the build skill, accept .agent/work-plans/

## Issue

https://github.com/rolker/geozui4d/issues/7

## Context

gz4d keeps project-local copies of four skills the agent workspace already
provides (`plan-task`, `research`, `review-pr`, `audit-project`). The
`build` skill is untracked because `.gitignore` `build/` matches
`.claude/skills/build/`. #10 and #14 already created `.agents/README.md`
and committed `.agent/work-plans/`. Owner decisions (2026-09-23, 2026-11-09
checkpoint): the conventions live in `.agents/README.md`; `AGENTS.md`
points at it and does not duplicate the rules; Ask First is granted for
`AGENTS.md`, `CLAUDE.md` and skill files, limited to what the issue and
Issue Review list; `research` is removed without waiting on
rolker/agent_workspace#331; #332 is closed, so the footprint dependency is met.

## Approach

One PR, atomic commits, in this order:

1. **Anchor `.gitignore` and track the build skill** — `build/` -> `/build/`,
   `install/` -> `/install/` (lines 2-3). Copy
   `SKILL.md` from the main checkout (`/home/roland/src/gz4d/.claude/skills/build/SKILL.md`)
   to the worktree and `git add` it. Check with `git check-ignore -v` that
   the skill is no longer ignored and `build/`, `install/` still are.
   Verified: no tracked path has a `build` or `install` component and no
   `install` path exists below the top level. Nested `build/` directories do
   exist in the main checkout (`libgz4d/cxx/build/`, `GeoZui4D/main/build/`)
   and stay ignored through their own nested `.gitignore` files
   (`libgz4d/.gitignore`, `GeoZui4D/main/.gitignore`, both `build/`), so
   anchoring is safe for that reason; only `.claude/skills/build/` changes
   state.
2. **Port the always-build audit step into the build skill** — add a
   Guidelines line to `.claude/skills/build/SKILL.md`: "Run the full build
   as part of any audit (a project audit always includes it)", taken from
   audit-project step 2. In the same edit, point the skill's commands at
   `scripts/build.sh` (the project's tracked build script: configures with
   `-DBUILD_GTESTS=ON`, builds all targets; tests run with
   `cd build && ctest --output-on-failure`), so "the full build" includes the
   test suite. The copied skill otherwise describes a build that skips the
   tests (it predates #9). Host decision, within "commit the build skill with
   the audit note". Separate commit so the port precedes the deletion.
3. **Delete the four duplicate skills** — `git rm -r` of
   `.claude/skills/{plan-task,research,review-pr,audit-project}`. Commit
   message notes the `research` gap is accepted by the owner and that
   the shadowing rationale was not verified (the earlier review found
   workspace descriptions shown), so the PR text must not claim it.
4. **Fix `brand-guidelines/SKILL.md` line 84** — change "(not `---` or `---`)"
   to "(not `---` or `—`)", matching the workspace copy; confirm with `diff`
   against `$WS_ROOT/.claude/skills/brand-guidelines/SKILL.md` that the
   file is then identical.
5. **Complete the conventions in `.agents/README.md`** — conventions 1-3 are
   already present (Common Pitfalls); only add or complete:
   - vendored code and the IVS files: append one `git log -- <path>`
     recency hint covering both lists (old audit-project section 6 checked
     both);
   - CMake: replace the `/review-pr ... issue #7 ports the rule` text with
     the rule itself: a change to any `CMakeLists.txt` gets a build-impact
     note in the PR, which also says whether dependencies are new or changed
     (old review-pr section 5);
   - new bullet: license-header coverage check, periodic and manual, across
     `.cpp/.h/.tcl`, classifying Apache 2.0 (ADR-0003) vs IVS vs none and
     excluding vendored code;
   - keep `std::filesystem` (ADR-0002) and IVS entries as they are.
6. **Remove stale text in `.agents/README.md`** — the `docs/work-plans/`
   layout entry is removed from under `docs/` and `.agent/work-plans/` is
   added at the top level of the layout diagram (tracked, one `issue-<N>/`
   directory per issue, plan and progress timeline); delete the
   "issue #7 tracks the fix" sentence in the build-dir bullet and make
   it say the ignore rule is `/build/`; delete the "project's own skills"
   bullet, or replace it with one line saying the loop skills come from the
   workspace. Also update "Every claim here was checked against the source on
   2026-11-02" to the implementation date, after verifying the new claims;
   keep the phrase "this guide stands on its own" verbatim (a roadmap check
   depends on it). (Commits 5 and 6 may be one if the diff is small; keep the
   convention additions and the stale-text removal in separate commits if not.)
7. **`AGENTS.md`** — keep Boundaries as is; add a short "Conventions" section
   with a pointer to `.agents/README.md` (without repeating the five rules)
   and the work-plans convention: each issue's plan and progress timeline are
   committed under `.agent/work-plans/issue-<N>/` on that issue's branch;
   the convention is provisional, pending rolker/agent_workspace#393, as the
   roadmap Deferral row says. One sentence, conditional: when this repository
   is worked from the agent workspace, the loop skills `plan-task`,
   `review-issue`, `review-plan`, `review-code`, `triage-reviews`,
   `address-findings`, `audit-project`, `run-issue`, `orchestrator`,
   `start-task`, `sitrep`, `onboard-project`, `document-project` and
   `test-engineering` come from it (verified from `session_scope: both` or
   `project` in `$WS_ROOT/.claude/skills/*/SKILL.md`); the workspace
   `research` skill does not reach project sessions (rolker/agent_workspace#331
   is open), so there is no research skill here for now. In the same commit,
   fix the duplicate `cd build` in the Build & Test block (lines 46-49: line
   46 already enters `build/`, so the install line should not `cd build`
   again), a host decision.
8. **`CLAUDE.md` Skills table** — drop `/plan-task`, `/review-pr`,
   `/audit-project`, `/research`; keep `/brand-guidelines` and `/build`;
   add one conditional line: when the repository is worked from the agent
   workspace, plan, review and audit are the workspace skills
   (`agent-workspace:plan-task`, `review-code`, `audit-project`); the
   workspace `research` skill is not available to project sessions yet
   (rolker/agent_workspace#331).
9. **`docs/roadmap.md` line 34** — reword the Direction text to "The skills
   that duplicate workspace skills are gone and the gz4d conventions live
   in `.agents/README.md`, with `AGENTS.md` pointing at them (#7)"; the
   check column stays `gh-issue 7 closed`.
10. **Verify** — grep the tree for remaining references to `/review-pr`,
    `/plan-task`, `/audit-project`, `/research`, `docs/work-plans`, and
    "issue #7"; run `cmake -S . -B build && cmake --build build --parallel 8`
    only if a source or CMake file changed (none is planned, so likely
    skipped and said so). Re-read the issue and compare. The PR description
    states the `research` gap and omits any unverified shadowing claim.

## Files to Change

| File | Change |
|------|--------|
| `.gitignore` | Anchor `/build/`, `/install/` |
| `.claude/skills/build/SKILL.md` | New tracked file; commands point at `scripts/build.sh`; add the audit-build line |
| `.claude/skills/{plan-task,research,review-pr,audit-project}/` | Deleted |
| `.claude/skills/brand-guidelines/SKILL.md` | Line 84 fix |
| `.agents/README.md` | Recency hint (vendored and IVS), CMake rule (with dependencies), license-header check; remove stale lines; `.agent/work-plans/` at top level of layout; bump verified date |
| `AGENTS.md` | Pointer to `.agents/README.md`; provisional work-plans convention; conditional workspace-skills sentence; fix duplicate `cd build` |
| `CLAUDE.md` | Skills table; conditional workspace line |
| `docs/roadmap.md` | Direction line 34 |

## Prior Art

The text of the deleted `review-pr` and `audit-project` skills is the
source for the rules; `.agents/README.md` already holds three of the five.
The workspace's `brand-guidelines` copy is the reference for step 4.

## Serves

The goal of staying usable with or without the workspace (`docs/roadmap.md`);
gz4d has no separate design document.

## Design Picture

No change to the design picture (gz4d keeps no design document).

## Principles Self-Check

| Principle | Consideration |
|---|---|
| A change includes its consequences | Steps 6, 8, 9 remove every statement the deletions make false |
| Put each thing at the level it applies to | Rules in `.agents/README.md` (where `review-code` reads); `AGENTS.md` only points (owner decision) |
| Instruction files are Ask First | Approval granted for this issue; edits limited to the items listed |
| Only what's needed | Only missing conventions are added; nothing re-written |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| gz4d 0002 std::filesystem | Yes | Already stated in `.agents/README.md`; kept, cited |
| gz4d 0003 Apache 2.0 | Yes | Names the header class in the license-coverage check |
| Workspace 0013 progress.md | Indirect | Work-plans convention mentions the progress timeline |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| Delete the skills | `CLAUDE.md` Skills table, `.agents/README.md` mentions | Yes (steps 6, 8) |
| `.gitignore` anchoring | Nested `build/` dirs (`libgz4d/cxx/build/`, `GeoZui4D/main/build/`) | Safe: they stay ignored via their own nested `.gitignore` files; only `.claude/skills/build/` changes state |
| Roadmap line | Sit-rep Direction check | Yes (step 9) |

## Open Questions

- None blocking. Whether to add a pre-commit guard for the vendored and IVS
  paths (Issue Review suggestion) stays a follow-up issue, out of scope.

## Estimated Scope

Single PR, about 8 small commits, no source or build-file changes.
