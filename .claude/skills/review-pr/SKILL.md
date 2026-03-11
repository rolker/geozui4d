---
name: review-pr
description: Evaluate a PR against project conventions and ADRs. Produces a structured report without posting unless asked.
---

# Review PR

## Usage

```
/review-pr <pr-number-or-url>
```

## Overview

Evaluate a pull request against project conventions and ADRs. Produces a
structured report in the conversation. Does not post comments or modify
the PR unless the user asks.

## Steps

### 1. Gather PR context

```bash
gh pr view <N> --json title,body,baseRefName,headRefName,files,additions,deletions,url,comments,reviews
gh pr diff <N>
```

Identify:
- What files changed
- The linked issue (if any)
- Whether a work plan exists (`docs/work-plans/PLAN_ISSUE-*.md`)

### 2. Load project context

- `AGENTS.md` -- project rules
- `docs/decisions/*.md` -- ADRs (scan titles, read those triggered by this change)

### 3. Evaluate against conventions

Check the PR against project rules:

- Does it modify vendored third-party code? (Togl, BWidget, RapidXML)
- Does it modify IVS copyright files without approval?
- Does it use `std::filesystem` instead of `boost::filesystem`?
- Are commits atomic (one logical change each)?
- Does it build successfully?

### 4. Check ADR compliance

For each ADR, assess whether it's triggered by this PR:
- Does the PR comply with the ADR's requirements?
- If not, what needs to change?

### 5. Check build impact

- Do the changes affect CMakeLists.txt files?
- Would the changes likely break the build?
- Are new dependencies introduced?

### 6. Produce the report

```markdown
## PR Review: #<N> -- <title>

**PR**: <url>
**Issue**: #<issue> (if linked)
**Files changed**: <count> (+<additions> -<deletions>)

### Convention Check

| Rule | Status | Notes |
|------|--------|-------|
| No vendored code changes | Pass/Concern | ... |
| std::filesystem usage | Pass/Concern | ... |
| Atomic commits | Pass/Concern | ... |
| ... | ... | ... |

### ADR Compliance

| ADR | Triggered | Compliant | Notes |
|-----|-----------|-----------|-------|
| ... | ... | ... | ... |

### Build Impact

<assessment of build impact>

### Summary

<1-3 sentence overall assessment>

### Recommended Actions

- [ ] <specific action items, if any>
```

## Guidelines

- **Report, don't act** -- output the review in the conversation. The user
  decides whether to post it.
- **Be specific** -- "Concern: modifies Togl/togl.c which is vendored code"
  is useful. "Watch: could be cleaner" is not.
- **Read the diff** -- don't just check file names. Read the actual changes.
- **Check the plan** -- if a work plan exists, compare the implementation
  against it. Note deviations.
