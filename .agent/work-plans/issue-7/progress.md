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
