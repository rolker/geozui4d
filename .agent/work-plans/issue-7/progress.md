---
issue: 7
---

# Issue #7 — Agent-workspace readiness

## Issue Review
**Status**: complete
**When**: 2026-11-09 11:49 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #7

### Scope Assessment

**Well-scoped?** Yes, as one PR of atomic commits, and smaller than the issue reads: #10 and #14 already did part of it (see "Already done"). What remains is the skill removals, the missing conventions, the `.gitignore`/build-skill fix and a `CLAUDE.md` refresh.
**Right repo?** Yes. Every change is in gz4d's own files.
**Serves**: roadmap Direction line "The skills that duplicate workspace skills are gone and the gz4d rules live in `AGENTS.md` (#7)", under the goal of staying usable with or without the workspace (`docs/roadmap.md`). gz4d has no separate design document.
**Design picture?** No change.
**Dependencies**: rolker/agent_workspace#332 (onboarding footprint question; the 2026-10-13 owner direction says the items that put workspace files into this repo wait on it) and #331 (`research`; the owner has already said not to wait for it). Check #332's state before starting.

### Already done (checked against the tree, 2026-11-09)

| Item | State |
|---|---|
| Rule 1, vendored code untouched | Done: `AGENTS.md` Never; `.agents/README.md` Common Pitfalls and layout. Missing: the "check recency with `git log -- <path>`" hint. |
| Rule 2, IVS files untouched | Done: `AGENTS.md` Never; `.agents/README.md`. |
| Rule 3, `std::filesystem` (ADR-0002) | Partly: `.agents/README.md` Common Pitfalls and `CLAUDE.md` Coding Conventions. Not in `AGENTS.md`. |
| Rule 4, build-impact note for CMake changes | Not done. `.agents/README.md` still says "issue #7 ports the rule to `AGENTS.md`". `AGENTS.md` only has top-level CMakeLists under Ask First. |
| Rule 5, license-header coverage check | Not done. |
| Build skill: "run the full build in any audit" | Not done, and `.claude/skills/build/` is not in this worktree at all because it is untracked and ignored. It exists only in the main checkout. |
| `.gitignore` `build/` over-match | Not done: line 2 `build/` and line 3 `install/` are unanchored. `/worktrees/` (line 63) is already there. |
| Tracked `.agent/work-plans/` | Done in practice: issue-8 and issue-9 are committed. The convention is not written in `AGENTS.md`. |
| `brand-guidelines` | Not done: L84 still differs from the workspace copy (broken em-dash row), so the "byte-identical" claim is wrong. |
| `CLAUDE.md` Skills table | Stale: still lists `/plan-task`, `/review-pr`, `/audit-project`, `/research` as project skills. |
| Five skills tracked | audit-project, brand-guidelines, plan-task, research, review-pr. The four duplicates are all still present. |

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Instruction files are Ask First | Action needed | The core work edits `AGENTS.md` and `CLAUDE.md`, both Ask First files in this project (`AGENTS.md` Boundaries). The owner must approve those edits before they are made; the skill files are covered by the same rule. |
| A change includes its consequences | Action needed | Remove the stale statements the removal makes false: `CLAUDE.md` Skills table; `.agents/README.md` lines on `/review-pr`, "The project's own skills", the `docs/work-plans/` layout entry and the "issue #7 tracks the fix" build-dir bullet; and `AGENTS.md`/`CLAUDE.md` mentions of build steps. |
| Put each thing at the level it applies to | Watch | The 2026-09-23 owner decision puts the rules in `.agents/README.md` with `AGENTS.md` pointing at it, while the roadmap line and the host brief say the rules live in `AGENTS.md`. Settle which: the lowest-cost reading is the short rule list in `AGENTS.md` (it is what every agent reads) with detail in `.agents/README.md`. |
| Only what's needed | OK | Removing the stale duplicates reduces context and ambiguity. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| gz4d 0002 (`std::filesystem`) | Yes | Cite it in the `AGENTS.md` rule. |
| gz4d 0003 (Apache 2.0) | Yes | Relevant to the license-header check wording. |
| Workspace 0013 (progress.md) | Indirect | Accepting `.agent/work-plans/` implies the progress timeline kept there. |

### Consequences

- `CLAUDE.md` Skills table; `.agents/README.md` statements named above.
- `.gitignore`: anchor `/build/` and `/install/`, then commit `.claude/skills/build/SKILL.md` (copy it from the main checkout; do not lose it).

### Recommendations

- Get the owner's yes to editing `AGENTS.md`/`CLAUDE.md`/skill files before starting, and confirm whether #332's footprint answer changes anything first.
- Add only the missing rules to `AGENTS.md`: `std::filesystem` (ADR-0002), the CMakeLists build-impact note for PRs, the periodic license-header check, the `git log -- <path>` recency hint, and one sentence naming the tracked `.agent/work-plans/` convention and the workspace as the source of the loop skills.
- Remove `.claude/skills/plan-task`, `research`, `review-pr`, `audit-project` (owner accepted the `research` gap); fix or delete `brand-guidelines` (L84).
- Anchor `/build/` and `/install/` in `.gitignore`, commit the `build` skill with the "run the full build in any audit" line.
- Update `CLAUDE.md` Skills table and the stale `.agents/README.md` statements in the same PR.
- Verify the shadowing claim or drop it from the PR description (already raised in the earlier review).

### Actions
- [ ] The core work edits `AGENTS.md` and `CLAUDE.md`, both Ask First files in this project (`AGENTS.md` Boundaries). The owner must approve those edits before they are made; the skill files are covered by the same rule.
- [ ] Remove the stale statements the removal makes false: `CLAUDE.md` Skills table; `.agents/README.md` lines on `/review-pr`, "The project's own skills", the `docs/work-plans/` layout entry and the "issue #7 tracks the fix" build-dir bullet; and `AGENTS.md`/`CLAUDE.md` mentions of build steps.
- [ ] Get the owner's yes to editing `AGENTS.md`/`CLAUDE.md`/skill files before starting, and confirm whether #332's footprint answer changes anything first.
- [ ] Add only the missing rules to `AGENTS.md`: `std::filesystem` (ADR-0002), the CMakeLists build-impact note for PRs, the periodic license-header check, the `git log -- <path>` recency hint, and one sentence naming the tracked `.agent/work-plans/` convention and the workspace as the source of the loop skills.
- [ ] Remove `.claude/skills/plan-task`, `research`, `review-pr`, `audit-project` (owner accepted the `research` gap); fix or delete `brand-guidelines` (L84).
- [ ] Anchor `/build/` and `/install/` in `.gitignore`, commit the `build` skill with the "run the full build in any audit" line.
- [ ] Update `CLAUDE.md` Skills table and the stale `.agents/README.md` statements in the same PR.
- [ ] Verify the shadowing claim or drop it from the PR description (already raised in the earlier review).

## Checkpoint
**Status**: complete
**When**: 2026-11-09 11:57 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: owner
**After**: issue-actions
**Decision**: proceed

Owner's answers at the issue-actions checkpoint (AskUserQuestion, 2026-11-09):

- Where the ported gz4d rules live: "Rules in .agents/README.md; AGENTS.md points". His 2026-09-23 decision on this issue stands ("The ported gz4d conventions go in .agents/README.md ... AGENTS.md points at it and doesn't duplicate the rules"); the roadmap Direction line "the gz4d rules live in AGENTS.md (#7)" is reworded to match, drafted from his words and listed for his veto: "The skills that duplicate workspace skills are gone and the gz4d conventions live in `.agents/README.md`, with `AGENTS.md` pointing at them (#7)".
- Ask First: "Yes, all of them". For this issue only, the plan and implementation may edit AGENTS.md (the pointer and the work-plans convention), CLAUDE.md (the Skills table) and the skill files (delete plan-task, research, review-pr, audit-project; commit the build skill with the audit note; fix the broken table row in brand-guidelines). Anything beyond what the issue and the Issue Review list comes back to him.

Dependency check by the host: rolker/agent_workspace#332 is closed and this project was onboarded through it (#10), so the 2026-10-13 direction that footprint items wait on #332 is satisfied; the footprint answers are in the tree (.agents/README.md exists, .agent/work-plans/ is committed). The research skill may be removed without waiting on rolker/agent_workspace#331 (owner, 2026-09-23 update).

## Plan Authored
**Status**: complete
**When**: 2026-11-09 11:58 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-7/plan.md` at `8679c61`

Nine-step plan in one PR: anchor .gitignore and track the build skill, port the audit-build line, delete the four duplicate skills, fix brand-guidelines, complete the conventions in .agents/README.md, point AGENTS.md at it with the work-plans convention, refresh CLAUDE.md and the roadmap line.

## Plan Review
**Status**: complete
**When**: 2026-11-09 12:01 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: needs-work

**Issue**: #7 — Agent-workspace readiness: remove duplicated skills, port gz4d conventions to AGENTS.md, track the build skill, accept .agent/work-plans/
**Plan**: `.agent/work-plans/issue-7/plan.md` at `8679c61`
**Branch**: `feature/issue-7`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | One PR of about 8 atomic commits, docs, skill files and `.gitignore` only. It is limited to what the owner approved at the checkpoint. |
| Issue alignment | Good | It covers all four issue sections and the owner decisions: rules in `.agents/README.md` with a pointer from `AGENTS.md`, `research` removed now, Direction line reworded. Serves: "The goal of staying usable with or without the workspace (`docs/roadmap.md`); gz4d has no separate design document." One Issue Review action is not addressed (finding 5). |
| File targeting | Good | Every stale reference the removals create is listed. A grep of `CLAUDE.md`, `AGENTS.md`, `.agents/README.md`, `README.md` and `docs/` for plan-task, research, review-pr, audit-project, docs/work-plans, "project's own skills" and "issue #7" finds only `CLAUDE.md` L85-88 and `.agents/README.md` L64, L173-174, L196, L210-214, plus roadmap L34. Steps 5, 6, 8 and 9 cover all of them. |
| Consequences | Needs work | The `.gitignore` row is wrong on the facts, though its conclusion holds (finding 3). |
| Principle alignment | Needs work | One planned sentence would document something false (finding 1). The plan says there are no tests because the change is docs-only, and it checks `.gitignore` with `git check-ignore`. Prior art and design picture are both stated. |
| ADR compliance | Good | gz4d ADR-0002 is kept and cited. ADR-0003 is named in the license-header check. Workspace ADR-0013 is covered by the work-plans convention. |
| ROS conventions | N/A | Not a ROS project. |

### Verified against the files

- **Ported content is complete.** Nothing gz4d-specific is lost. The audit-project license-header step (§1) goes to step 5, the always-build step (§2) to step 2, and the vendored `git log` recency check (§5) to step 5. review-pr §3 is already in `.agents/README.md` (vendored, IVS, `std::filesystem`) and its §5 build impact goes to step 5. plan-task and research hold no gz4d rules. research's only gz4d-specific item is `docs/research_digest.md`, which was never created.
- **`brand-guidelines`.** After step 4 the file is byte-identical to `$WS_ROOT/.claude/skills/brand-guidelines/SKILL.md` (clean on the workspace `main`). `diff` shows L84 as the only difference.
- **`.gitignore` anchoring is safe.** I applied the anchored rules to the main checkout. The nested `libgz4d/cxx/build/`, `GeoZui4D/main/build/` and the build dirs under the dead dirs stay ignored through `libgz4d/.gitignore:3` and `GeoZui4D/main/.gitignore:1` (both `build/`), and through the root's dead-directory entries. The only path whose state changes is `.claude/skills/build/`. No path named `install` exists below the top level, and no tracked path has a `build` or `install` component.
- **Roadmap line.** A scratch copy with the step 9 wording, run through `sitrep_checks.sh --root .`, parses as a `line` row with check `gh-issue 7 closed` and leaves every other row unchanged.
- **Build skill in the main checkout.** Its last change is 2026-03-11, before #9 and #14. It says nothing false: `mkdir -p build && cd build && cmake .. && make -j$(nproc)` still builds, and `install/bin/GeoZui4D` is still correct. It does not mention `scripts/build.sh`, the gtest suite or `ctest` (finding 4).

### Findings

1. **[Principle alignment, must-fix]** Step 7 would write into `AGENTS.md` that "the loop skills (plan, review, audit, research) come from the agent workspace". That is false today. rolker/agent_workspace#331 is open, and the workspace `research` skill has no `session_scope`, so it does not reach gz4d sessions; the owner accepted that gap. This breaks AGENTS.md Never "Document from assumptions". Drop `research` from the list, or say it is unavailable until #331 lands. Phrase the sentence as conditional ("when this repository is worked from the agent workspace, ..."), because the project stands without the workspace (`.agents/README.md` L12-17) and its registration is local to one machine. The same conditional wording applies to the step 8 `CLAUDE.md` line.
2. **[Issue alignment, suggestion]** The work-plans convention in step 7 should agree with the roadmap Deferral row, which says the location is "the workspace's provisional default" until rolker/agent_workspace#393. Say it is provisional or point at that row, so `AGENTS.md` does not state a permanent rule that the roadmap marks as open.
3. **[Consequences, suggestion]** Correct the `.gitignore` Consequences row and the step 1 "verified" note. Nested build dirs do exist in the main checkout (`libgz4d/cxx/build/`, `GeoZui4D/main/build/`). They stay ignored through the nested `.gitignore` files' own `build/` rules, so the change is safe for that reason and not because none exist. Without the correction, the PR text may repeat the wrong claim.
4. **[File targeting, suggestion]** The build skill copied in step 1 predates #9. It configures without `-DBUILD_GTESTS=ON` and runs no tests, so step 2's "run the full build as part of any audit" points at a build that skips the test suite that `scripts/build.sh` builds. Changing the skill beyond the audit note is outside the owner's grant, so ask him at implementation, or open a follow-up, rather than editing it silently.
5. **[Issue alignment, suggestion]** The Issue Review action item also lists "`AGENTS.md`/`CLAUDE.md` mentions of build steps". The plan neither handles it nor says why it is dropped. Those steps are still accurate, apart from `AGENTS.md` L49's second `cd build` after L46 has already entered `build/`. Editing them is outside the owner's grant, so record it as out of scope or a follow-up in the PR's gaps list.
6. **[Issue alignment, suggestion]** audit-project §6 also checked IVS files with `git log`. Step 5 adds the recency hint only for vendored code. Make the one sentence cover both lists.
7. **[Issue alignment, suggestion]** review-pr §5 also asked "are new dependencies introduced?" Step 5's CMake rule ("build-impact note") should name new or changed dependencies, so that part of the ported check is not lost.
8. **[File targeting, suggestion]** In step 6, the `.agent/work-plans/` entry belongs at the top level of the layout diagram, not inside the `docs/` subtree where `docs/work-plans/` sits now (L64). Also update `.agents/README.md`'s "checked against the source on 2026-11-02" date (L19), since steps 5 and 6 add new claims. Keep the "this guide stands on its own" phrase intact, because Direction line 5 checks for it.

### Summary

The plan carries out the owner's decisions correctly and lists every stale reference. The `.gitignore`, brand-guidelines and roadmap changes all check out against the files. One planned sentence would write a false claim about `research` into `AGENTS.md`. Fix that, and it is ready; the other findings can be folded into implementation.

### Recommended Actions

- [ ] Reword step 7 (and the step 8 `CLAUDE.md` line) so it does not claim `research` comes from the workspace, and phrase both conditionally on the workspace being used.
- [ ] Mark the work-plans convention as provisional per the roadmap Deferral row (rolker/agent_workspace#393).
- [ ] Correct the `.gitignore` Consequences row: nested build dirs exist but stay ignored through the nested `.gitignore` files.
- [ ] Ask the owner whether the build skill should run `scripts/build.sh` and `ctest`, or open a follow-up.
- [ ] Record the `AGENTS.md`/`CLAUDE.md` build-step item (including the double `cd build` at L46-49) as out of scope or a follow-up.
- [ ] Extend the `git log` recency hint to the IVS files, and include new dependencies in the CMake build-impact rule.
- [ ] Place `.agent/work-plans/` at the top level of the layout diagram, and bump the guide's verified date.

## Checkpoint
**Status**: complete
**When**: 2026-11-09 12:02 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: host (plan review verdict needs-work with one must-fix wording correction and seven suggestions, none changing scope; /orchestrator's table lets the host answer revise at plan)
**After**: plan
**Decision**: revise

Plan Review 14c7620 on plan 8679c61. The plan-task re-run takes the must-fix (AGENTS.md and CLAUDE.md must not claim the research skill reaches gz4d sessions from the workspace while rolker/agent_workspace#331 is open; the "loop skills come from the workspace" sentences apply only when the repository is worked from the workspace) and all seven suggestions: the work-plans convention is marked provisional pending rolker/agent_workspace#393, matching the roadmap deferral; the .gitignore reasoning is corrected (libgz4d/cxx/build/ and GeoZui4D/main/build/ exist and stay ignored through their own nested .gitignore files); the git-log recency hint covers the IVS files too; the build-impact rule also asks about new or changed dependencies; .agent/work-plans/ sits at the top level of the layout diagram, the "checked against the source" date is updated, and the phrase "this guide stands on its own" is kept.

Two host decisions, listed for the owner's veto: (a) the build skill, when committed, has its commands point at scripts/build.sh, the project's tracked build script, so the "full build in any audit" line includes the test suite; this stays within "commit the build skill with the audit note" since the skill otherwise describes a build that skips the tests the project now has. (b) The duplicate `cd build` in AGENTS.md lines 46-49 is fixed in the same AGENTS.md commit; the Issue Review listed the build-step text and the owner approved AGENTS.md edits for the items the review lists.

## Plan Authored
**Status**: complete
**When**: 2026-11-09 12:03 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-7/plan.md` at `48e7b4d`

Revision after plan review 14c7620: conditional workspace-skill wording (research gap named), provisional work-plans convention, corrected .gitignore reasoning, build skill pointed at scripts/build.sh, cd build fix in AGENTS.md, IVS recency hint, dependency rule, README layout and date.

## Plan Review
**Status**: complete
**When**: 2026-11-09 12:06 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: ready

**Issue**: #7 — Agent-workspace readiness: remove duplicated skills, port gz4d conventions to AGENTS.md, track the build skill, accept .agent/work-plans/
**Plan**: `.agent/work-plans/issue-7/plan.md` at `48e7b4d`
**Branch**: `feature/issue-7`

Round 2. This round checks that the revision carries the round-1 Plan Review (14c7620: one must-fix, seven suggestions) and the host's two decisions (build skill points at `scripts/build.sh`; duplicate `cd build` fixed in `AGENTS.md`), and that it adds no new problem. The owner's checkpoint decisions are not re-opened.

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | Unchanged: one PR of about 8 commits, docs, skill files and `.gitignore` only, within the owner's Ask First grant plus the two host decisions listed for his veto. |
| Issue alignment | Good | All eight round-1 items are carried (table below). Serves: "The goal of staying usable with or without the workspace (`docs/roadmap.md`); gz4d has no separate design document." |
| File targeting | Good | The same stale references are covered. Two leftovers are described in findings 2 and 3. |
| Consequences | Good | The `.gitignore` row is now correct. In this worktree `git check-ignore -v` gives `.gitignore:2:build/` for the skill. `libgz4d/.gitignore` and `GeoZui4D/main/.gitignore` each carry `build/`. |
| Principle alignment | Good | The false `research` claim is gone. Step 6 still offers an unconditional alternative (finding 1). |
| ADR compliance | Good | Unchanged: gz4d 0002 and 0003 are cited, and workspace 0013 is covered by the work-plans convention. |
| ROS conventions | N/A | Not a ROS project. |

### Round-1 items, checked against the revised plan

| # | Item | Carried? |
|---|---|---|
| 1 (must-fix) | No false claim about `research`; workspace wording conditional | Yes. Steps 7 and 8 are conditional ("when this repository is worked from the agent workspace") and name the `research` gap with rolker/agent_workspace#331. The step 7 list of 14 skills matches every skill with `session_scope: both` or `project` in `$WS_ROOT/.claude/skills/*/SKILL.md` (workspace `main`, clean). `research` has no `session_scope`. |
| 2 | Work-plans convention provisional | Yes. Step 7 cites rolker/agent_workspace#393 and the roadmap Deferral row, which exists at `docs/roadmap.md` L40. |
| 3 | `.gitignore` reasoning | Yes. Step 1 and the Consequences row now name the nested build dirs and the nested `.gitignore` files. |
| 4 | Build skill vs test suite | Yes, by host decision (a). The plan's account of `scripts/build.sh` is accurate: `-DBUILD_GTESTS=ON`, builds all targets, tests via `cd build && ctest --output-on-failure`. |
| 5 | Build-step text in `AGENTS.md`/`CLAUDE.md` | Partly. `AGENTS.md` is fixed by host decision (b), but `CLAUDE.md` is not mentioned (finding 2). |
| 6 | Recency hint covers IVS files | Yes (step 5). |
| 7 | Dependencies in the CMake build-impact rule | Yes (step 5). |
| 8 | Layout diagram, verified date, "this guide stands on its own" | Yes (step 6). The phrase matches the roadmap L33 check verbatim. |

### Findings

1. **[Principle alignment, suggestion]** Step 6 still lets the implementer replace the `.agents/README.md` "project's own skills" bullet (L210-214) with "one line saying the loop skills come from the workspace". This is the unconditional wording that round 1 removed from steps 7 and 8, and it would go into the guide that "stands on its own" (L12-17). Delete the bullet, or use step 7's conditional wording with the `research` gap.
2. **[Issue alignment, suggestion]** `CLAUDE.md` Build Commands (L20-29) has the same duplicate `cd build` as `AGENTS.md`. Its Run line, `./install/bin/GeoZui4D`, is also wrong once the shell is in `build/`. Because `CLAUDE.md` imports `AGENTS.md`, the two blocks will disagree after step 7. Fix it in step 8 on the same footing as host decision (b), or list it as a gap in the PR.
3. **[File targeting, suggestion]** Two small items:
   - Step 2 should say what the skill's `--install` mode becomes. `scripts/build.sh` does not install, so the install step needs the repo-root form `cmake --install build --prefix install`.
   - `.agents/README.md` L7-10 says "The rules agents must follow are in `AGENTS.md` ... This file says what the code is and where to look". Once the conventions live in the guide (the reworded Direction line), that text is wrong. Adjust it in step 6.
4. **[Principle alignment, suggestion]** Step 10 skips the build because no source or CMake file changes. Step 2 does rewrite the build skill's commands, though. Run the rewritten commands once (`scripts/build.sh --parallel 8`, then `cd build && ctest --output-on-failure`), so the skill is checked before it is committed.

### Summary

The revision carries the must-fix and all seven round-1 suggestions, and it implements both host decisions accurately. The four remaining items are wording and verification details that can be handled during implementation. The plan is ready.

### Recommended Actions

- [ ] Step 6: delete the "project's own skills" bullet, or reword it conditionally like step 7.
- [ ] Fix or record the `CLAUDE.md` Build Commands block (duplicate `cd build`, Run path).
- [ ] State the build skill's install command, and adjust `.agents/README.md` L7-10 to match the new split.
- [ ] Run the rewritten build skill's commands once before committing it.

## Checkpoint
**Status**: complete
**When**: 2026-11-09 12:05 -05:00
**By**: Claude Code Agent (claude-fable-5-1)
**Decided-by**: host (round-2 plan review verdict ready, no must-fix; /orchestrator's table lets the host answer proceed at plan)
**After**: plan
**Decision**: proceed

Round-2 Plan Review b2f1037 on plan 48e7b4d: ready; the 14 skills named in step 7 match the workspace's session_scope frontmatter; the research gap is stated. Four suggestions go into the implement handoff rather than a third plan round: the "project's own skills" bullet in .agents/README.md is deleted or reworded conditionally like step 7, never an unconditional "the loop skills come from the workspace"; the CLAUDE.md Build Commands block (lines 20-29) gets the same duplicate-cd-build fix as AGENTS.md and its run line made correct from the repository root (host decision, listed for the owner's veto: CLAUDE.md imports AGENTS.md and the two blocks would otherwise disagree; CLAUDE.md edits are approved for this issue); the build skill's install command becomes the repo-root form `cmake --install build --prefix install` since scripts/build.sh does not install, and .agents/README.md lines 7-10, which say the rules are in AGENTS.md, are corrected to match the owner's decision; the build skill's commands are run once before the skill is committed, with 8 jobs.

## Implementation
**Status**: complete
**When**: 2026-11-09 12:13 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Branch**: feature/issue-7 at `726f2cf`

Nine commits, one per plan step (steps 5 and 6 kept separate):

- `fbc31c1` `.gitignore` anchors `/build/` and `/install/`; the build skill is copied from the main checkout and tracked. `git check-ignore -v`: `build/`, `install/` ignored by the root rules, `libgz4d/cxx/build/` and `GeoZui4D/main/build/` by their nested `.gitignore` files, the skill no longer ignored.
- `9ed93bd` Build skill runs `scripts/build.sh` then `(cd build && ctest --output-on-failure)`; `--install` uses `cmake --install build --prefix install`; Guidelines gain "Run the full build as part of any audit" and a note on running the script's two cmake steps by hand with `--parallel 8` to limit jobs.
- `2c3f134` Deletes `.claude/skills/{plan-task,research,review-pr,audit-project}`.
- `3b2005b` `brand-guidelines` line 84 fixed; `cmp` with the workspace copy: identical.
- `234f754` `.agents/README.md` Common Pitfalls: `git log --oneline -5 -- <path>` recency check for vendored and IVS paths; CMake build-impact note in the PR including new or changed dependencies; periodic manual license-header check (Apache 2.0 / IVS / none, vendored excluded).
- `cf7323b` `.agents/README.md` stale text: opening lines now say the conventions are in this guide and `AGENTS.md` points here; `.agent/work-plans/` at top level of the layout, `docs/work-plans/` removed; build-dir bullet gives `/build/`; "project's own skills" bullet deleted (not reworded); checked-on line now "2026-11-02, and the conventions and layout changes added since on 2026-11-09" (only the new claims were re-verified). "this guide stands on its own" kept verbatim.
- `f81975d` `AGENTS.md` Conventions section (pointer, provisional work-plans convention citing rolker/agent_workspace#393, conditional list of the 14 workspace loop skills, `research` gap with rolker/agent_workspace#331); Build & Test: no second `cd build`, run line `cd .. && ./install/bin/GeoZui4D [files...]`.
- `771155f` `CLAUDE.md` Skills table reduced to `/brand-guidelines` and `/build` plus the conditional workspace line; Build Commands block made identical to `AGENTS.md`'s.
- `726f2cf` Roadmap Direction line reworded exactly as the owner's wording; check unchanged.

Verification:
- Build, run by hand with 8 jobs (not `scripts/build.sh`): `cmake -S . -B build -DBUILD_GTESTS=ON && cmake --build build --parallel 8` succeeded (exit 0; 467 `warning:` lines, mostly the Boost bind deprecation message and existing code warnings); `(cd build && ctest --output-on-failure)`: 1/1 passed (`test_libgz4d`); `cmake --install build --prefix install`: succeeded, `install/bin/GeoZui4D` present.
- `sitrep_checks.sh --root .`: every row parses; the #7 line reports `false` (issue still open), all others unchanged.
- `git grep` for plan-task, research, review-pr, audit-project, docs/work-plans, "project's own skills", "issue #7" over `CLAUDE.md`, `AGENTS.md`, `.agents/README.md`, `README.md`, `docs/`: the only hits are the intended workspace-skill and research-gap sentences in `AGENTS.md` and `CLAUDE.md`, and unrelated uses of "research" in `README.md` and ADR-0003.
- No file under `libgz4d/`, `GeoZui4D/` or any `CMakeLists.txt` changed.

Departure: two commit subjects were lost through a shell heredoc mistake; the two local, unpushed commits were rebuilt with `git commit-tree` with the intended messages (no hooks are installed, so no check was bypassed).

### Decision summary
- Conventions live in `.agents/README.md`; `AGENTS.md` points at them and does not repeat them (owner).
- The four duplicate skills are removed; the workspace `research` skill does not reach project sessions while rolker/agent_workspace#331 is open, stated in `AGENTS.md` and `CLAUDE.md` (owner accepted the gap). No claim about skill shadowing is made.
- The build skill runs `scripts/build.sh` plus ctest, so the audit's full build includes the tests (host decision, for the owner's veto).
- The duplicate `cd build` is fixed in both `AGENTS.md` and `CLAUDE.md`, whose build blocks now match (host decision, for the owner's veto).
- The work-plans location is recorded as provisional pending rolker/agent_workspace#393.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-09 12:19 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: changes-requested

**Branch**: feature/issue-7 at `4d974c0`
**Base**: main
**Depth**: Deep (reason: 13 files, +575/-470; AGENTS.md, CLAUDE.md and skill files are governance override triggers)
**Must-fix**: 1 | **Suggestions**: 5
**Round**: 1 | **Ship**: continue — round 1: 1 must-fix; first round always re-reviews after fixes

Specialists: static analysis (no profile for `.md`/`.gitignore`, so not checked), governance, plan drift, Claude adversarial (fresh subagent), cross-model Gemini (3 suggestions, all dropped below) and Codex (no issues), both EXIT=0.

Verified by the lead: no `.pre-commit-config.yaml`, hooks dir holds only `*.sample`, `core.hooksPath` unset, so the `git commit-tree` rebuild bypassed nothing; 9ed93bd, 2c3f134 and their rebuilt descendant 3b2005b have their subjects, `Refs #7`, the Co-Authored-By trailer and the agent identity (one cosmetic extra trailing newline). `brand-guidelines/SKILL.md` is byte-identical to the workspace copy. The 14 skills named in `AGENTS.md` are exactly those with `session_scope: both|project`; `research` has none and rolker/agent_workspace#331 is open. `sitrep_checks.sh --root .` parses every row; the Direction line is the owner's wording verbatim; "this guide stands on its own" survives. `.gitignore`: `git check-ignore --no-index` shows `libgz4d/cxx/build/**` and `GeoZui4D/main/build/**` still ignored by the nested `.gitignore` files, `main_refactor/build` and `ng/build` by the root dead-dir rules; README's sub-project builds are out of tree. AGENTS.md/CLAUDE.md build blocks work pasted in order. Nothing gz4d-specific from the four deleted skills is lost except the exact IVS file list (suggestion 3). Configured `build/` has `BUILD_GTESTS=ON` and `ctest -N` lists `test_libgz4d`.

Dropped as false positives: adversarial "109 files become untracked under nested build dirs" (its check used `--exclude-from` with the root file only; the nested `.gitignore` files cover them); Gemini ".hpp missing from the license check" (all four `.hpp` are vendored RapidXML); Gemini "CLAUDE.md build block is drift" (host checkpoint approved it); CLAUDE.md IVS "with explicit approval" vs AGENTS.md Never (pre-existing, outside the owner's grant).

### Findings
- [x] (must-fix) Tracked build skill hardcodes `--parallel 8`, contradicting the owner's #9 decision that published commands say the core count (7182a5f); 8 jobs is the agents' local rule. Use a placeholder, e.g. `cmake --build build --parallel <N>` (or `scripts/build.sh --parallel <N>`: the script appends its args and the last `--parallel` wins, checked on CMake 3.28); not `$(nproc)`, which defeats a bullet about limiting jobs — `.claude/skills/build/SKILL.md:57-58`
- [x] (suggestion) "(a project audit always includes it)" is false now: the workspace `audit-project` that replaces gz4d's runs no build; drop the parenthetical — `.claude/skills/build/SKILL.md:56`
- [x] (suggestion) Name the five IVS files the deleted audit-project listed (`Sv_Matrix4.{h,cpp}`, `Sv_Quaternion.{h,cpp}`, `Sv_Raster.h`); "Sv_Raster" also covers `Sv_Raster.cpp`, which is CCOM Apache-2.0, and the new recency hint says "each IVS file" with no list — `.agents/README.md:190-195`
- [x] (suggestion) Skill description still says "Build and optionally install ... verify compilation"; align with the tests it now runs and CLAUDE.md's "Build, test and optionally install" — `.claude/skills/build/SKILL.md:3`
- [x] (suggestion) `cmake --install` needs CMake 3.15, above the 3.14 minimum (ADR-0001); new in the skill, pre-existing in `AGENTS.md:70`, `CLAUDE.md:25`, `.agents/README.md:140`; note it or open a follow-up — `.claude/skills/build/SKILL.md:45`
- [x] (suggestion) AGENTS.md/CLAUDE.md build blocks configure without gtests and run no tests, unlike the build skill and guide (Gemini and Claude adversarial both raised it); outside the owner's grant, so list it as a gap or follow-up in the PR description — `AGENTS.md:63-74` (deferred: editing the AGENTS.md and CLAUDE.md build blocks further is outside the owner's grant; listed as a gap for the PR description in the Implementation entry's Decision summary)

## Implementation
**Status**: complete
**When**: 2026-11-09 12:24 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**Branch**: feature/issue-7 at `ab5da91`
**Addressed**: Local Review (Pre-Push) at `4d974c0` (2026-11-09 12:19 -05:00)
**Commits**: `103b7e0`, `01f6350`, `600c85d`, `783e2f5`, `cbcc645`, `ab5da91`

### Actions
- [x] (must-fix) Hardcoded `--parallel 8` replaced by `scripts/build.sh --parallel <N>` (the script passes its arguments to the build step; the last `--parallel` wins) and a sentence to pick N for the machine — `.claude/skills/build/SKILL.md:60-63` (`103b7e0`)
- [x] (suggestion) "(a project audit always includes it)" dropped; the rule now reads as this project's own: full build, with the tests, in any audit — `.claude/skills/build/SKILL.md:58` (`01f6350`)
- [x] (suggestion) Guide names the five IVS files (`Sv_Matrix4.h`, `Sv_Matrix4.cpp`, `Sv_Quaternion.h`, `Sv_Quaternion.cpp`, `Sv_Raster.h`), says `Sv_Raster.cpp` is CCOM Apache-2.0 and not one of them, and the recency hint refers to "the five IVS files above". Checked against the files: the first four carry the IVS copyright; `Sv_Raster.h` has no copyright header at all (the guide says so) — `.agents/README.md:190-198` (`600c85d`)
- [x] (suggestion) Description now "Build, test and optionally install GeoZui4D ... verify that it compiles and the tests pass", matching CLAUDE.md — `.claude/skills/build/SKILL.md:3` (`783e2f5`)
- [x] (suggestion) CMake 3.15 note for `cmake --install` added to the skill and as a trailing comment in the guide's by-hand block; AGENTS.md and CLAUDE.md left unchanged (Ask First files; the owner's grant did not cover it) — `.claude/skills/build/SKILL.md:48`, `.agents/README.md:140` (`cbcc645`)
- [x] (suggestion) AGENTS.md/CLAUDE.md build blocks without gtests or a test step — `AGENTS.md:63-74` (deferred: editing those blocks further is outside the owner's grant; listed below as a gap for the PR description) (`ab5da91`)

Verification: `grep -n parallel .claude/skills/build/SKILL.md` shows only `<N>`, no number; `sitrep_checks.sh --root .` exit 0, all nine rows parse (the #7 line `false` while the issue is open). No build: text-only changes. No file under `libgz4d/`, `GeoZui4D/` or any `CMakeLists.txt` changed.

### Decision summary
- Conventions live in `.agents/README.md`; `AGENTS.md` points at them and does not repeat them (owner).
- The four duplicate skills are removed; the workspace `research` skill does not reach project sessions while rolker/agent_workspace#331 is open, stated in `AGENTS.md` and `CLAUDE.md` (owner accepted the gap). No claim about skill shadowing is made.
- The build skill runs `scripts/build.sh` plus ctest, so the audit's full build includes the tests (host decision, for the owner's veto); its job-limit hint uses a placeholder `<N>`, not a machine-specific number.
- The duplicate `cd build` is fixed in both `AGENTS.md` and `CLAUDE.md`, whose build blocks now match (host decision, for the owner's veto).
- The work-plans location is recorded as provisional pending rolker/agent_workspace#393.
- Gap for the PR description: the AGENTS.md and CLAUDE.md build blocks still omit -DBUILD_GTESTS=ON and the test step; a follow-up, since editing them further is outside this issue's approval.
- Gap for the PR description: the same two blocks' `cmake --install` needs CMake 3.15, above the 3.14 minimum (ADR-0001), unnoted there for the same reason; and their IVS lines (`AGENTS.md` Never, `CLAUDE.md` IVS section) still say "Sv_Raster", which also covers the Apache-2.0 `Sv_Raster.cpp` (pre-existing).

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-09 12:25 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Dispatch**: resumed (agent a4aa2ca832f3801bc, resume 1 of 3)
**Verdict**: changes-requested

**Branch**: feature/issue-7 at `c38ac4d`
**Base**: main
**Depth**: Deep (reason: whole-diff classification, 13 files; governance files; fix round reviewed on the fix diff `4d974c0..ab5da91`)
**Must-fix**: 1 | **Suggestions**: 2
**Round**: 2 | **Ship**: recommended — round 2: 1 mechanical must-fix (prev 1), not rising — fix and ship rather than another full round

Specialists: governance and plan drift (lead), Claude adversarial (fresh subagent), cross-model Gemini (3 suggestions, all dropped) and Codex (no issues), both EXIT=0; static analysis not applicable (`.md` only).

Round-1 items checked against the files: the must-fix is fixed (no job count in the skill; `scripts/build.sh --parallel <N>`, and the script does append "$@" after its own `--parallel`, last wins); the audit rule no longer claims a workspace behaviour; the description matches CLAUDE.md; the 3.15 note is in the skill and the guide's by-hand block; the AGENTS.md/CLAUDE.md test-step gap is in the Decision summary as a PR-description gap. AGENTS.md, CLAUDE.md and the roadmap are untouched this round; `sitrep_checks.sh --root .` exits 0 with all nine rows. Fix commits carry the agent identity and the trailer.

Dropped: Gemini's stateful `cd` in the AGENTS.md/CLAUDE.md blocks (labelled per line, owner-scoped, already a deferred gap); Gemini's "IVS list departs from the plan" (it answers the round-1 review); Gemini's `git log` baseline wording (faithful port of the old audit text, unchanged in substance this round).

### Findings
- [x] (must-fix) The guide now says `Sv_Raster.cpp` "is not one of them", narrowing the AGENTS.md/CLAUDE.md Never rule ("Sv_Raster", no extension) on header evidence that does not hold: `Sv_Raster.cpp` never had an IVS line, and its CCOM Apache-2.0 header came from the blanket relicensing commit 11a46d7, not from an owner ruling. Drop that sentence and list `Sv_Raster.cpp` with the others as covered by the Never rule (the conservative reading), leaving whether it is truly IVS for the owner (it arose from the round-1 suggestion's wording) — `.agents/README.md:190-195`
- [x] (suggestion) The layout comment still calls all `Sv_*` files "IVS-copyright files", which contradicts the explicit list; Sv_Pointer, Sv_Slider3, Sv_Standards and Sv_Widgets carry CCOM Apache-2.0. Point at the list instead — `.agents/README.md:56`
- [x] (suggestion) "as part of any audit: this project's rule" reads as a fragment; e.g. "Run the full build, with the tests, as part of any audit of this project" — `.claude/skills/build/SKILL.md:58-59`

## Implementation
**Status**: complete
**When**: 2026-11-09 12:28 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Dispatch**: resumed (agent aedb35908270466e6, resume 1 of 3)

**Branch**: feature/issue-7 at `384b735`
**Addressed**: Local Review (Pre-Push) at `c38ac4d` (2026-11-09 12:25 -05:00)
**Commits**: `3cfbde3`, `edba0a2`, `384b735`

### Actions
- [x] (must-fix) Guide lists six protected files (`Sv_Matrix4.h`, `Sv_Matrix4.cpp`, `Sv_Quaternion.h`, `Sv_Quaternion.cpp`, `Sv_Raster.h`, `Sv_Raster.cpp`), the conservative reading of the AGENTS.md Never entry "Sv_Raster". Header facts stated neutrally and checked: the four Matrix4/Quaternion files carry the IVS line; `Sv_Raster.h` has no copyright header; `Sv_Raster.cpp`'s CCOM Apache-2.0 header came from `11a46d7` (2026-03-11, relicensing), and `git log -p` finds no IVS line in its history. The Sv_Raster origin is left to the owner; "is not one of them" dropped; recency hint says "the six files above" — `.agents/README.md:190-203` (`3cfbde3`)
- [x] (suggestion) Layout comment now "LightDirector and the Sv_* widgets; six Sv_* files are protected (see Common Pitfalls)"; checked that Sv_Pointer, Sv_Slider3, Sv_Standards and Sv_Widgets carry the Apache-2.0 header — `.agents/README.md:56` (`edba0a2`)
- [x] (suggestion) Audit rule reads "Run the full build, with the tests, as part of any audit of this project" — `.claude/skills/build/SKILL.md:58` (`384b735`)

Verification: `sitrep_checks.sh --root .` exit 0, all nine rows parse; "this guide stands on its own" still verbatim at `.agents/README.md:16`. No build: text-only changes.

### Decision summary
- Conventions live in `.agents/README.md`; `AGENTS.md` points at them and does not repeat them (owner).
- The four duplicate skills are removed; the workspace `research` skill does not reach project sessions while rolker/agent_workspace#331 is open, stated in `AGENTS.md` and `CLAUDE.md` (owner accepted the gap). No claim about skill shadowing is made.
- The build skill runs `scripts/build.sh` plus ctest, so the audit's full build includes the tests (host decision, for the owner's veto); its job-limit hint uses a placeholder `<N>`, not a machine-specific number.
- The duplicate `cd build` is fixed in both `AGENTS.md` and `CLAUDE.md`, whose build blocks now match (host decision, for the owner's veto).
- The work-plans location is recorded as provisional pending rolker/agent_workspace#393.
- The guide treats six widget files as protected, `Sv_Raster.cpp` included, reading the Never entry "Sv_Raster" conservatively; whether the two `Sv_Raster` files share the IVS origin is the owner's to settle (host decision, for the owner's veto).
- Gap for the PR description: the AGENTS.md and CLAUDE.md build blocks still omit -DBUILD_GTESTS=ON and the test step; a follow-up, since editing them further is outside this issue's approval.
- Gap for the PR description: the same two blocks' `cmake --install` needs CMake 3.15, above the 3.14 minimum (ADR-0001), unnoted there for the same reason.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-09 12:29 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Dispatch**: resumed (agent a4aa2ca832f3801bc, resume 2 of 3)
**Verdict**: changes-requested

**Branch**: feature/issue-7 at `23b0a72`
**Base**: main
**Depth**: Deep (reason: round 3 convergence check per handoff)
**Must-fix**: 1 | **Suggestions**: 1
**Round**: 3 | **Ship**: recommended — round 3: 1 mechanical must-fix (prev 1), not rising — fix and ship rather than another full round

Scope: the round-2 findings and the fix diff `c38ac4d..384b735`. Cross-model: skipped (round-3 convergence check, per handoff). Governance and plan drift by the lead; Claude adversarial (fresh subagent) ran, as the skill keeps the non-cross-model specialists at round 3.

Round-2 findings, checked against the files: the six protected files are listed at `.agents/README.md:190-201` with each header fact true (IVS line in the four Matrix4/Quaternion files; none in `Sv_Raster.h`; `Sv_Raster.cpp`'s Apache-2.0 header from `11a46d7`; `git log -p --follow` finds no IVS line in it) and the Sv_Raster origin left to the owner; the layout line at :56 points at the list; `SKILL.md:58` reads as one sentence. Fix commits carry the agent identity and trailer; `sitrep_checks.sh --root .` exits 0.

### Findings
- [ ] (must-fix) New text "Never modify the IVS files ... without the owner's approval" turns the AGENTS.md Never hard stop (AGENTS.md:27, 33-34, no approval path) into an Ask First rule while citing Never; the old guide text had no approval clause. Drop "without the owner's approval" (the CLAUDE.md "without explicit approval" conflict stays pre-existing and the owner's) — `.agents/README.md:190-191`
- [ ] (suggestion) "no IVS line appears anywhere in its history" holds only for git history since the 2026-02-27 import (e76de6a); say so, and note the file's "Mark Paton - various additions & mods." credit (Paton is the author of the IVS Sv_Matrix4/Sv_Quaternion files) as why the origin is open — `.agents/README.md:198`
