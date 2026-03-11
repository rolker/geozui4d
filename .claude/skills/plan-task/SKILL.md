---
name: plan-task
description: Generate a work plan for an issue. Saves to docs/work-plans/ and commits as the first step on the feature branch.
---

# Plan Task

## Usage

```
/plan-task <issue-number>
```

## Overview

Generate a work plan for an issue with ADR awareness. Use EnterPlanMode for
general codebase exploration and approach design; use this skill to produce a
structured, committed plan that accounts for project conventions and ADR
compliance.

## Steps

### 1. Read the issue

```bash
gh issue view <N> --json title,body,labels,comments,url
```

### 2. Load project context

- `AGENTS.md` -- project rules
- `docs/decisions/*.md` -- ADR titles (read triggered ADRs in full)

### 3. Explore the codebase

Read relevant files to understand the current state:
- Files that will be modified
- Tests that exist for those files
- Documentation that references them

### 4. Generate the plan

Write the plan to `docs/work-plans/PLAN_ISSUE-<N>.md`:

```markdown
# Plan: <issue-title>

## Issue

<issue URL>

## Context

<Brief summary of current state and what needs to change>

## Approach

1. **<step>** -- <what and why>
2. ...

## Files to Change

| File | Change |
|------|--------|
| `path/to/file` | Description of change |

## ADR Compliance

| ADR | Triggered | How addressed |
|-----|-----------|---------------|
| <relevant ADR> | Yes/No | <explanation> |

## Open Questions

- <anything that needs human input before implementation>

## Estimated Scope

<single PR / multiple PRs / needs breakdown>
```

### 5. Commit the plan

```bash
git add docs/work-plans/PLAN_ISSUE-<N>.md
git commit -m "Add work plan for #<N>

<one-line summary of the approach>"
```

### 6. Create a draft PR

```bash
git push -u origin HEAD
ISSUE_TITLE=$(gh issue view <N> --json title --jq '.title')
gh pr create --draft --title "$ISSUE_TITLE" --body-file docs/work-plans/PLAN_ISSUE-<N>.md
```

### 7. Report to user

Summarize:
- What the plan proposes
- Which ADRs were considered
- Any open questions that need input
- Link to the draft PR

## Guidelines

- **Concrete, not generic** -- "Update tests" is not a plan step. "Add test
  for edge case X in `test_foo.cpp`" is.
- **Flag conflicts early** -- if the plan would violate an ADR, say so and
  propose an alternative.
- **Include open questions** -- if the approach depends on a choice the user
  should make, list it.
- **Keep it short** -- a plan should be 30-80 lines. If longer, the task may
  need to be broken down.
