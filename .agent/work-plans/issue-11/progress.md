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
   can redirect them. Changed in the startup paragraph and in the Common
   Pitfalls note on the install step.
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
- **Defects found later**: none recorded yet.
- **Redone because the first result was not what was meant**: none recorded yet.
