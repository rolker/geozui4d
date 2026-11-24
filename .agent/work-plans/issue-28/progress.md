---
issue: 28
---

# Issue #28 — Research: datum realizations, epochs and plate motion (PROJ as the authority?)

## Research
**Status**: complete
**When**: 2026-11-24 13:52 to 14:03 -05:00
**By**: Claude Code Agent (claude-fable-5-2), a background agent following the workspace's research skill

The first entries of the project's research digest,
`.agents/workspace-context/research_digest.md`: six sections, one per question
in the issue, each finding marked verified, sourced or inferred. No source
code changed.

The launching session re-ran the comparison of PROJ 9.7.1 with the Helmert
implementation named in the digest, and the no-epoch, WGS 84 ensemble and
epoch-change cases; all four reproduced. The other findings were not
re-checked.

The digest's entries are longer than the research skill's usual size, on the
owner's word ("push as is").

Not done, as the digest says where it applies: NGS HTDP was not run; the ITRF
parameter tables were read from PROJ's copy, not at their origin; the RTCM
standard was not read; several statements about positioning services rest on
secondary sources.

## Trial counts

Light-level trial (no review loop), piece of work 2 (the owner counted it,
2026-11-24).

- **Start**: 2026-11-24 13:52 -05:00 (worktree created).
- **Merge**: on the owner's word, 2026-11-24; the time is in the merge record on pull request 29 (this file is committed before the merge).
- **Defects found later**: none recorded yet.
- **Redone because the first result was not what was meant**: none recorded yet.
