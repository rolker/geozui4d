---
issue: 11
---

# Issue #11 — Agent guide: four descriptive details deferred from the review of #10

## Implementation
**Status**: complete
**When**: started 2026-11-24 14:53 -05:00
**By**: Claude Code Agent (claude-fable-5-2)

Worked directly in the worktree, without the review loop (the owner's
light-level trial). Documentation only: `.agents/README.md`. Each point was
re-verified against the source at `b3d3974` before editing.

1. **Libraries the app needs although CMake looks them up as optional: no
   change needed.** The Dependencies table already lists PortAudio and
   Bluetooth for `GeoZui4D/main` (added with #16). The OpenNI part of the
   issue is out of date: #16 guarded the Kinect code with `GZ4D_KINECT`
   (`engine/Scene3D.cpp`, `objects/CMakeLists.txt`), and the guide already
   lists OpenNI as an optional feature.
2. **"Only work installed" now says "by default".** The default, `-wall` and
   `-cp` start scripts set `appdir` and then append the common part that
   sources the `GeoZui4D.conf` files (`MainWindow.cpp`), so a config file
   can redirect them. Changed in the startup paragraph and in the Build &
   Test note on the install step.
3. **`scene` is a header, not a subdirectory.** `libgz4d/cxx/include/gz4d/`
   has 39 subdirectories; the guide names 22 of them, now called a partial
   list. `scene.h`, `Spaceball.h`, `Wiimote.h` and `Kinect.h` are listed as
   single headers.
4. **`gzmod/` is partly installed.** `gzmod/CMakeLists.txt` installs 12 of the
   22 top-level `.py` files and the `ais/`, `deltaT/` and `lighting/`
   directories, and `gzmod/gz4d/CMakeLists.txt` installs part of `gz4d/`;
   `simMBFish/` is not installed. The layout line says so.

### Not done

The issue expects more slips of the same kind in the guide. None was looked
for beyond the four points.

## Trial counts

Light-level trial (no review loop), piece of work 3.

- **Start**: 2026-11-24 14:53 -05:00 (worktree created).
- **Merge**: on the owner's word; the time is in the merge record on the
  pull request (this file is committed before the merge).
- **Review**: one round, 14:56:24 to 14:58:18 -05:00 (about 2 minutes), run at
  the owner's request before the merge; the fixes were pushed by 14:59:54.
- **Defects found by the review, before the merge**: 3 must-fix (see Local
  Review).
- **Defects found later**: none recorded yet.
- **Redone because the first result was not what was meant**: none recorded yet.

## Local Review
**Status**: complete
**When**: 2026-11-24 14:58 -05:00
**By**: Claude Code Agent (claude-fable-5-2)
**Verdict**: changes-requested

**PR**: #31 at `579231b`
**Depth**: Standard (reason: 77 changed lines, in the 50 to 199 band)
**Must-fix**: 3 | **Suggestions**: 6

Review started 14:56:24 and ended 14:58:18 -05:00 (about 2 minutes): a Claude
adversarial subagent, Gemini and Codex through `cross_model_review.sh`, and the
lead's governance pass. No linter profile covers Markdown; no work plan exists.
All three independent reviewers flagged the first finding or its neighbours
(Gemini without access to the source, as a claim to verify).

### Findings
- [x] (must-fix) `-cp` does not run from an install: `GeoCoastPilot.tcl` and the five scripts it sources are not installed, so "by default those modes only work installed" is false for it (Claude adversarial, Codex) — `.agents/README.md:101`
- [x] (must-fix) "By default the install step is required to run the app" is too broad: `-nav` reads `GeoNav.tcl` from the working directory and `-cp` cannot run installed (Claude adversarial) — `.agents/README.md:196`
- [x] (must-fix) "parts of gz4d/" hides that `gz4d/base` is installed to `gzmod/base`, not under `gzmod/gz4d/` (Claude adversarial; Codex as a suggestion) — `.agents/README.md:63`
- [x] (suggestion) progress.md names the wrong section: the second edit is under Build & Test, not Common Pitfalls (Claude adversarial) — `.agent/work-plans/issue-11/progress.md:27`
- [x] (suggestion) Only the `.py` files of `ais/`, `deltaT/` and `lighting/` are installed (Claude adversarial, Gemini) — `.agents/README.md:64`
- [x] (suggestion) With `appdir` set, `-nav` still sources `GeoNav.tcl` from the working directory (Claude adversarial, Codex) — `.agents/README.md:106`
- [x] (suggestion) "The header subdirectories are the modules" is stated before the single headers are named; the line after is not wrapped (Gemini, Claude adversarial) — `.agents/README.md:75`
- [x] (suggestion) An `appdir` override must hold bitmaps and BWidget as well as the scripts (Gemini) — `.agents/README.md:196`
- [ ] (suggestion) `README.md` still says "The install step is required" without the qualifier (governance) — `README.md:181` — not taken: the user README describes the default mode, where the statement holds

### After the review

The three must-fix findings and five of the six suggestions were fixed in
`2b432d5`, each new statement checked against the source
(`visualizer.tcl`, `GeoCoastPilot.tcl`, `GeoNav.tcl`, the `install` rules in
`GeoZui4D/main/CMakeLists.txt`, `scripts/CMakeLists.txt` and
`gzmod/gz4d/CMakeLists.txt`). The fixes were not re-reviewed: the owner asked
for one review and the merge.

What the review changed: the unreviewed version said the `-cp` mode works
when installed, which is false; all three independent reviewers pointed at
it. The implementation had recorded "none was looked for" about further
slips of this kind.

## Merge (report-only)
**Status**: complete
**When**: 2026-11-24 15:00 -05:00
**By**: merge_pr.sh (Claude Code Agent)

**PR**: #31 at `b56e0f5`
**Mode**: enforce
**Scope**: project
**Conditions**: latest Local Review entry is at `579231b`, not the PR head `b56e0f5` (stale review: touches `.agents/README.md`, which is not a merge-time document file)
