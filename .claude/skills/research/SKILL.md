---
name: research
description: Research a topic using web sources and maintain a living research digest at docs/research_digest.md.
---

# Research

## Usage

```
/research <topic>
/research --ingest <url>
/research --refresh
```

## Overview

Research external sources on a topic and maintain a living digest. The digest
captures best practices, emerging techniques, and relevant developments for
the gz4d project.

## Digest

**Location**: `docs/research_digest.md` (create if it doesn't exist)

### Format

```markdown
# Research Digest

<!-- Last updated: YYYY-MM-DD -->

## <Topic Title>

**Added**: YYYY-MM-DD | **Sources**: [link1](url), [link2](url)

Key takeaways:
- <concise finding>
- <concise finding>

**Relevance**: <why this matters to gz4d>

---
```

## Workflow

### Adding research (`/research <topic>`)

1. Create a feature branch: `git checkout -b research/<topic-slug>`
2. Search the web for current information on the topic
3. Read and synthesize relevant sources
4. Check the existing digest for related entries -- update rather than duplicate
5. Append or update the entry in `docs/research_digest.md`
6. Update the "Last updated" timestamp
7. Commit: `git add docs/research_digest.md && git commit -m "research: <topic>"`
8. Push and create a PR: `git push -u origin HEAD && gh pr create --fill`

### Ingesting a URL (`/research --ingest <url>`)

1. Create a feature branch
2. Fetch and read the URL content
3. Extract key takeaways relevant to gz4d
4. Append to the digest
5. Commit, push, create PR

### Refreshing (`/research --refresh`)

1. Create a feature branch
2. Read the digest
3. For each entry, check if it's still current
4. Update, mark as stale, or remove entries as needed
5. Update timestamps
6. Commit, push, create PR

## Guidelines

- **External sources only** -- this skill surveys the web, papers, and docs.
  It does not scan the gz4d codebase.
- **Concise entries** -- each topic should be 5-15 lines.
- **Cite sources** -- every entry needs at least one link.
- **Dedup** -- check for existing entries before adding. Update rather than
  duplicate.
- **Staleness** -- entries older than 90 days without a refresh should be
  flagged for review.
