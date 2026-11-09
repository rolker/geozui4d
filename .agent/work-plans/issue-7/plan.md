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
   `install/` -> `/install/` (lines 2-3). Verified: no `build`/`install`
   directory is tracked or present below the top level in the tree. Copy
   `SKILL.md` from the main checkout (`/home/roland/src/gz4d/.claude/skills/build/SKILL.md`)
   to the worktree and `git add` it. Check with `git check-ignore -v` that
   the skill is no longer ignored and `build/`, `install/` still are.
2. **Port the always-build audit step into the build skill** — add a
   Guidelines line to `.claude/skills/build/SKILL.md`: "Run the full build
   as part of any audit (a project audit always includes it)", taken from
   audit-project step 2. Separate commit so the port precedes the deletion.
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
   - vendored code: append the `git log -- <path>` recency hint;
   - CMake: replace the `/review-pr ... issue #7 ports the rule` text with
     the rule itself: a change to any `CMakeLists.txt` gets a build-impact
     note in the PR;
   - new bullet: license-header coverage check, periodic and manual, across
     `.cpp/.h/.tcl`, classifying Apache 2.0 (ADR-0003) vs IVS vs none and
     excluding vendored code;
   - keep `std::filesystem` (ADR-0002) and IVS entries as they are.
6. **Remove stale text in `.agents/README.md`** — the `docs/work-plans/`
   layout entry becomes `.agent/work-plans/` (tracked, one `issue-<N>/`
   directory per issue, plan and progress timeline); delete the
   "issue #7 tracks the fix" sentence in the build-dir bullet and make
   it say the ignore rule is `/build/`; delete the "project's own skills"
   bullet, or replace it with one line saying the loop skills come from the
   workspace. (Commits 5 and 6 may be one if the diff is small; keep the
   convention additions and the stale-text removal in separate commits if not.)
7. **`AGENTS.md`** — keep Boundaries as is; add a short "Conventions" section
   with a pointer to `.agents/README.md` (without repeating the five rules)
   and the work-plans convention: each issue's plan and progress timeline are
   committed under `.agent/work-plans/issue-<N>/` on that issue's branch;
   the loop skills (plan, review, audit, research) come from the agent
   workspace, not from this repository.
8. **`CLAUDE.md` Skills table** — drop `/plan-task`, `/review-pr`,
   `/audit-project`, `/research`; keep `/brand-guidelines` and `/build`;
   add a line saying plan, review and audit come from the workspace
   (`agent-workspace:plan-task`, `review-code`, `audit-project`).
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
| `.claude/skills/build/SKILL.md` | New tracked file; add the audit-build line |
| `.claude/skills/{plan-task,research,review-pr,audit-project}/` | Deleted |
| `.claude/skills/brand-guidelines/SKILL.md` | Line 84 fix |
| `.agents/README.md` | Recency hint, CMake rule, license-header check; remove stale lines (layout, build-dir bullet, skills bullet) |
| `AGENTS.md` | Pointer to `.agents/README.md`; work-plans convention |
| `CLAUDE.md` | Skills table |
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
| `.gitignore` anchoring | Sub-project builds in a nested `build/` are no longer ignored | Noted as a risk; none exist in the tree |
| Roadmap line | Sit-rep Direction check | Yes (step 9) |

## Open Questions

- None blocking. Whether to add a pre-commit guard for the vendored and IVS
  paths (Issue Review suggestion) stays a follow-up issue, out of scope.

## Estimated Scope

Single PR, about 8 small commits, no source or build-file changes.
