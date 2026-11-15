---
issue: 20
---

# Issue #20 — Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader

## Issue Review
**Status**: complete
**When**: 2026-11-15 09:56 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #20

### Scope Assessment

**Well-scoped?** Yes. One registration line plus a check. Small enough for one PR.
**Right repo?** Yes. `GdalRaster.cpp` is in GeoZui4D. Writing GeoTIFF from ice_cube (the owner's option 3) is ice_cube's business and is not part of this issue.
**Serves**: the project roadmap goal (use GeoZui4D with the file formats people use today, `docs/roadmap.md`). The project has no design document ("none").
**Design picture?** No change. It adds an alias to an existing loader.
**Dependencies**: none identified. Open issues #1, #2, #3 and #6 are unrelated.

Verified against source. `GdalRaster.cpp:20-22` registers `tif`, `tiff` and `vrt` only. No other class registers `asc` (searched `GeoZui4D` and `libgz4d`). `Scene3D.cpp` falls back to the texture loader when `Load()` returns nonzero. NODATA cells become NaN at `GdalRaster.cpp:154`, and `buildVertexBuffer` skips NaN cells (line ~283), so the NODATA acceptance line is already implemented once the file is routed to the loader. The placement code needs the CRS to come from GDAL's `.prj` handling. If the AAIGrid driver does not read the WKT2 `.prj`, `spatialReference` stays empty and the grid would be placed as if its metre coordinates were lon/lat. I could not check this here: `gdalinfo` and the Python bindings are not installed (`gdal-config` says 3.8.4). The check has to run in the test itself.

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Only what's needed | OK | The issue is a direct owner request and the change is one line. |
| Test what you change | Action needed | GeoZui4D is a GUI app, so the acceptance cannot be checked at a screen without a person. Add a small gtest (the project already has `-DBUILD_GTESTS=ON` and ctest) or a standalone program. It should load a synthetic `.asc` and `.prj` through the same calls `GdalRaster::Load` makes (`GDALOpen`, `GetProjectionRef`, `SetFromUserInput`, `gz4d::geo::Projection`). It should assert three things: the registry maps `asc` to `GdalRaster` (not the texture path), the orthographic origin projects to its latitude and longitude, and the NODATA cell comes out NaN. |
| Private data stays private | Action needed | The test data must be synthetic: a few-cell grid with a made-up origin, generated in the test or committed as a tiny fixture. Nothing from the owner's ice_cube data. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| 0001 CMake 3.14 and C++17 | Watch | Only if the test needs a new CMake target. The top-level `CMakeLists.txt` is "ask first", but a test target in a sub-CMakeLists should not need that. |
| 0002 std::filesystem | No | |
| 0003 Apache 2.0 | Watch | A new test file needs the SPDX header. |

### Consequences

- If a test target is added, the build script and README's test steps already run ctest. Confirm the new test is picked up.
- No documentation lists the supported surface extensions (not checked beyond a search of `docs/`), so none needs updating.

### Recommendations

- Register `.asc` as `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` beside the existing aliases.
- Add the automated check described above. Make it fail first if GDAL does not read the WKT2 `.prj`, since that is the real risk to the placement acceptance.
- If the `.prj` is not read, fall back to reading the sidecar `.prj` in `GdalRaster` for that case. Raise it as a scope question to the owner rather than widening this issue unasked.
- Keep GeoTIFF output out of this issue (ice_cube's own business).

### Actions
- [ ] GeoZui4D is a GUI app, so the acceptance cannot be checked at a screen without a person. Add a small gtest (the project already has `-DBUILD_GTESTS=ON` and ctest) or a standalone program. It should load a synthetic `.asc` and `.prj` through the same calls `GdalRaster::Load` makes (`GDALOpen`, `GetProjectionRef`, `SetFromUserInput`, `gz4d::geo::Projection`). It should assert three things: the registry maps `asc` to `GdalRaster` (not the texture path), the orthographic origin projects to its latitude and longitude, and the NODATA cell comes out NaN.
- [ ] The test data must be synthetic: a few-cell grid with a made-up origin, generated in the test or committed as a tiny fixture. Nothing from the owner's ice_cube data.
- [ ] Register `.asc` as `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` beside the existing aliases.
- [ ] Add the automated check described above. Make it fail first if GDAL does not read the WKT2 `.prj`, since that is the real risk to the placement acceptance.
- [ ] If the `.prj` is not read, fall back to reading the sidecar `.prj` in `GdalRaster` for that case. Raise it as a scope question to the owner rather than widening this issue unasked.
- [ ] Keep GeoTIFF output out of this issue (ice_cube's own business).

## Checkpoint
**Status**: complete
**When**: 2026-11-15 09:57 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (the Actions are how to verify the acceptance, not a scope change; the one scope question is conditional)
**After**: issue-actions
**Decision**: proceed

The Actions ask for a gtest (or stand-alone program) on synthetic data that checks the .asc registration, the placement from an orthographic WKT2 .prj, and NODATA as NaN, written so that it fails first if GDAL's AAIGrid driver does not read the WKT2 .prj. If it does not, a sidecar .prj fallback in GdalRaster would widen the issue: that is the owner's question, raised only if the test shows it is needed. The plan should take the narrow scope and say what it does if the .prj is not read (stop and report, not add the fallback).

## Plan Authored
**Status**: complete
**When**: 2026-11-15 09:58 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-20/plan.md` at `a4fce1b`

Register `.asc` for GdalRaster; add a libgz4d gtest on a synthetic WKT2-.prj grid that fails first if GDAL ignores the .prj (then stop and report, no fallback), plus a source-level ctest for the registration.

## Plan Review
**Status**: complete
**When**: 2026-11-15 10:01 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: ready

**Issue**: #20 — Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader
**Plan**: `.agent/work-plans/issue-20/plan.md` at `a4fce1b`
**Branch**: `feature/issue-20`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | One alias line, one gtest, gtest CMake wiring. Narrow scope from the Checkpoint is kept: synthetic data only, stop and report if the WKT2 `.prj` is not read, no sidecar fallback. |
| Issue alignment | Good | Covers all three acceptance lines and the issue-review Actions (synthetic data, fails first on the `.prj`, alias line, GeoTIFF out). Serves (checked): "The project roadmap goal of using GeoZui4D with file formats people use today (`docs/roadmap.md`). The project has no design document: none." `docs/roadmap.md:5` states that goal. |
| File targeting | Needs work | `check_asc_registered.cmake` and its `add_test` should go (finding 1). `README.md` is missing (finding 2). |
| Consequences | Needs work | `README.md:78-84` "Supported File Formats" lists extensions; the issue review searched only `docs/` and missed it (finding 2). |
| Principle alignment | Good, with finding 1 | Test-first on the real GDAL/PROJ calls is the right check of the one real risk. The source-grep ctest fails the "only what's needed" / "enforce as simply as possible" test (finding 1). |
| ADR compliance | Good | 0001: only `libgz4d/cxx/gtest/CMakeLists.txt` changes, top level untouched. 0002: `std::filesystem` for the temp dir. 0003: SPDX header on the new file. |
| ROS conventions | N/A | Not a ROS project. |

### Findings

1. **[Principle alignment — the planner's open question]** Drop step 4 (the `cmake -P` grep of `GdalRaster.cpp`). It fails only if someone deletes the very line this PR adds, so it checks the diff, not the behaviour: it does not exercise the registry, `getRegistryEntryFromExt` (`Object.cpp:1797-1809`, which matches `'.'+extension` exactly, case-sensitive like `tif`), or `Scene3D` routing. Nothing better is within reach at reasonable cost: the app is one `add_executable(GeoZui4D ${GEOZUI4D_SRC})` (`GeoZui4D/main/CMakeLists.txt:303`) with no objects library, and `GdalRaster.cpp` pulls in `MainWindow`/`Scene3D`/GL, so a registry test means restructuring the app build, well beyond this issue. The alias is a one-liner identical in form to the `tiff` and `vrt` lines beside it: review covers it, and the owner's at-screen open is the runtime check. Say so in the PR. Remove `check_asc_registered.cmake` from Files to Change.
2. **[Consequences]** Add `.asc` to `README.md`'s "Supported File Formats" (bathymetry list, line 80). The plan's consequences row says "No document lists them (searched `docs/`)"; the README does.
3. **[Approach — suggestion]** Axis order in the placement assertion. Nothing in libgz4d or the app sets an axis-mapping strategy (no `SetAxisMappingStrategy` / `OAMS_TRADITIONAL_GIS_ORDER` anywhere), and with GDAL 3.8.4 a transform to `SetWellKnownGeogCS("WGS84")` returns latitude in x (checked with a throwaway program: UTM 19N (500000, 4800000) -> x=43.35, y=-69.0). So `Projection(srs, wgs84)(0, 0)` gives `Point[0]` = lat, `[1]` = lon, the opposite of the comment at `GdalRaster.cpp` ("[0]=lon and [1]=lat"). Write the expectation explicitly against lat in `[0]` (or assert both orderings are distinguished, e.g. with an origin where |lat| != |lon|, which 12.3456 / -45.6789 already gives) so the test is not quietly written to whatever comes out. This does not misplace a projected grid today: `autoProjection` (`Object.cpp:515-535`) uses the object's own spatial reference when it has one and ignores the swapped centre, so it is no reason to widen this issue; mention the misleading comment in the PR as a possible follow-up rather than fixing it here.
4. **[Scope — suggestion]** The "100 m east" second point is optional; the origin check and a projected, non-geographic CRS already show the `.prj` was read. Keep it only if it stays a few lines.

### Summary

The plan is sound and narrow, and its gtest checks the one real risk (GDAL reading the WKT2 `.prj`) with the same calls `GdalRaster::Load` makes. Drop the source-grep ctest, add `.asc` to the README format list, and pin the axis order in the placement assertion; none of these needs a re-plan.

### Recommended Actions

- [ ] Drop step 4 and `libgz4d/cxx/gtest/check_asc_registered.cmake`; state in the PR that the alias is verified by review and the owner's at-screen open.
- [ ] Add `.asc` to the bathymetry list in `README.md` "Supported File Formats".
- [ ] In `gdal_asc.cpp`, assert the origin with latitude in `Projection` output `[0]` and longitude in `[1]` (GDAL 3 authority order, no axis-mapping strategy set); note the misleading `GdalRaster.cpp` comment in the PR as a follow-up.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 10:01 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (plan review verdict ready; its two must-fix items are implementation instructions, no re-plan needed)
**After**: plan
**Decision**: proceed

The Plan Review at c679309 is ready. To the implement pass: drop step 4 and check_asc_registered.cmake (the alias is checked by code review and by the owner opening a file at a screen; the PR says so); add .asc to README.md:78-84 "Supported File Formats"; pin the axis order in the placement assertion (with GDAL 3.8.4, Projection's WGS84 output [0] is latitude) and note in the PR that GdalRaster.cpp's comment "[0]=lon and [1]=lat" reads backwards, as a possible follow-up, without changing it here; the second "100 m east" point is optional.

## Implementation
**Status**: partial
**When**: 2026-11-15 10:04 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Branch**: feature/issue-20 at `db55e60`

Stopped at the plan's stop condition: GDAL 3.8.4's AAIGrid driver does not read a WKT2 `.prj`. No sidecar `.prj` fallback was added (the owner's call).

**What changed** (commit `db55e60`):
- `libgz4d/cxx/gtest/gdal_asc.cpp` (new, SPDX header): writes a 4x3 `.asc` (10 m cells, NODATA -9999 in one cell) and a WKT2 `.prj` copied in form from `LocalFrame.wkt2()` (ellipsoidal orthographic, EPSG method 9840) about a made-up origin (lat 12.3456, lon -45.6789), in a temp dir removed afterwards. It makes the calls `GdalRaster::Load` makes and asserts: AAIGrid opens it, non-byte band, geotransform, non-empty `GetProjectionRef()`, a projected CRS, the origin at lat in `[0]` and lon in `[1]` within 1e-9 deg (axis order pinned, per the review), a point 100 m east, and the NODATA cell mapped to NaN with the comparison `Load` uses, the other cells finite.
- `libgz4d/cxx/gtest/CMakeLists.txt`: adds `gdal_asc.cpp` to `test_libgz4d` only `if(GDAL_FOUND)`. Build impact: one extra source in the existing gtest executable when `BUILD_GTESTS=ON` and GDAL is found (it already links `${LIBS}`, which holds GDAL); no new target, top-level `CMakeLists.txt` untouched.
- **Not done**, deliberately: the `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc")` line and the README "Supported File Formats" entry. With the `.prj` unread, registering `.asc` would place WKT2 grids as if their metres were degrees. Plan step 4 / `check_asc_registered.cmake` dropped per the checkpoint; when the alias goes in, it is checked by code review and by the owner opening a file at a screen.

**Verification** (built in the session scratchpad with `-DBUILD_GTESTS=ON`, `--parallel 4`; test target only, the app was not built since `GdalRaster.cpp` is unchanged):
- `test_libgz4d`: 23 of 24 tests pass; `GdalAscTest.ReadsWkt2PrjPlacementAndNoData` fails at its first guard, `gdal_asc.cpp:123`: `GetProjectionRef()` is `""`. ctest therefore reports `test_libgz4d` failed: the branch is red on purpose until the owner decides.
- `CPL_DEBUG=ON` shows the cause: `AAIGrid: Loaded SRS from .../grid.prj` then `OGR_ESRI: Can't find Projection`. The driver passes the `.prj` to `importFromESRI`, which takes ESRI WKT1 only.
- A throwaway probe (GDAL 3.8.4, removed afterwards): the same grid with a WKT1 `.prj` of the same orthographic gives a 368-character `GetProjectionRef()`; with the WKT2 `.prj`, length 0. `OGRSpatialReference::SetFromUserInput` on the same WKT2 string succeeds (err 0, projected), so a sidecar read in `GdalRaster` would work if the owner wants that.
- The placement and NODATA assertions were not reached with WKT2.

**Owner's decision needed**, one of: (a) a sidecar `.prj` fallback in `GdalRaster::Load` when `GetProjectionRef()` is empty (`SetFromUserInput` on the file's text); (b) ice_cube writes the WKT1 form beside the `.asc` (`LocalFrame.wkt1()` exists) or GeoTIFF (his option 3); (c) both. With (b) only, the test's `.prj` would change to WKT1 and the alias and README line go in.

**Possible follow-up** (not changed here): the comment in `GdalRaster.cpp` before the centre conversion, "[0]=lon and [1]=lat", reads backwards: with GDAL 3 and no axis-mapping strategy, `Projection`'s WGS84 output has latitude in `[0]`.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 10:07 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: owner
**After**: phase-failed
**Decision**: restart
**Phase**: implement

The implement pass stopped (partial): GDAL 3.8.4's AAIGrid driver reads a .prj as ESRI WKT1 only, so a WKT2 .prj (what ice_cube writes, LocalFrame.wkt2()) leaves the grid with no CRS; a WKT1 .prj of the same projection is read. Asked "How should the CRS reach GeoZui?" with the options "ice_cube writes WKT1 in .prj", "GeoZui reads WKT2 .prj itself" (when GDAL finds no CRS, GdalRaster reads the sidecar .prj with SetFromUserInput), "Both" (ice_cube switches to WKT1 and GeoZui also gets the fallback for other WKT2 .prj files) and "Stop #20 for now", the owner answered: "Both".

For #20 that adds the GeoZui half: a sidecar .prj fallback in GdalRaster when GDAL finds no CRS. The ice_cube half (write WKT1 in its .prj) is done from an ice_cube session, not here.

## Issue Review
**Status**: complete
**When**: 2026-11-15 10:17 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)

**Issue**: #20

### Scope Assessment

**Well-scoped?** Yes. Still one PR: the `.asc` alias, a sidecar `.prj` fallback of a few lines in `GdalRaster::Load`, the README format entry, and the test already on the branch. The issue body predates the restated goal and says nothing of the fallback; this comment and the owner's restart comment carry it (the body is not edited).
**Right repo?** Yes. `GeoZui4D/main/objects/GdalRaster.cpp` is in this repo. The ice_cube half (write WKT1 in `.prj`) belongs to ice_cube and is not part of this issue.
**Serves**: the roadmap goal (use GeoZui4D with the file formats people use today, `docs/roadmap.md`). The project has no design document ("none").
**Design picture?** No change. It adds an alias and a fallback inside an existing loader.
**Dependencies**: none identified. The ice_cube WKT1 change is independent: with the fallback, WKT2 `.prj` files from ice_cube today open too.

Verified against source. At `GdalRaster.cpp:79-82` the CRS comes only from `ds->GetProjectionRef()`; when it is empty, `spatialReference` stays empty, and `:93` then treats the centre as geographic, so a metre grid is placed as if in degrees. That is the failure the branch's gtest (`db55e60`) reproduces (GDAL 3.8.4 AAIGrid reads the `.prj` as ESRI WKT1 only; `SetFromUserInput` accepts the WKT2, per the 10:04 timeline entry). The fallback therefore fits at line 81: if the string is empty, read the `.prj` beside the dataset and call `SetFromUserInput` on its text. Points to settle in the plan: derive the `.prj` path from the filename with the extension swapped (`std::filesystem`, ADR-0002), try `.prj` and `.PRJ` since GDAL's own lookup does, and print a message to `std::cerr` (as the loader already does) when the file exists but does not parse, so a failed CRS is not silent.

### Principle Alignment

| Principle | Status | Notes |
|---|---|---|
| Only what's needed | OK | The fallback is the owner's decision ("Both"); keep it to the empty-CRS case and do not rewrite the CRS GDAL did find. |
| Test what you change | Action needed | The existing gtest stops at GDAL's own reading and does not run the fallback. `GdalRaster.cpp` cannot be linked into a gtest (it pulls in `MainWindow`/`Scene3D`/GL; see the plan review), so the fallback code would be untested as written. Put the sidecar read in a small function in libgz4d (for example a helper that returns a `gz4d::ogr::SpatialReference` from a raster path and the dataset's own WKT), call it from `GdalRaster::Load`, and have `gdal_asc.cpp` call that same helper: the test then passes on the WKT2 `.prj` and still asserts placement (origin at lat `[0]`, lon `[1]`) and NODATA. Add a second case with a WKT1 `.prj` showing GDAL's own path is untouched, and one with no `.prj` showing the loader still fails cleanly rather than guessing. |
| Private data stays private | OK | The test data is synthetic (made-up origin 12.3456 / -45.6789), as the earlier review asked. Keep it so. |
| Put each thing at the level it applies to | OK | The WKT2 reading belongs here (it helps any WKT2 `.prj`, not only ice_cube's); the WKT1 writing belongs in ice_cube. |

### ADR Applicability

| ADR | Triggered | Notes |
|---|---|---|
| 0001 CMake 3.14 and C++17 | Watch | Only if the helper needs a new source in a `CMakeLists.txt`; libgz4d's sub-CMakeLists is not "ask first", the top-level one is. |
| 0002 std::filesystem | Triggered | Build the `.prj` path with `std::filesystem`, not string slicing or boost. |
| 0003 Apache 2.0 | Watch | A new header/source file needs the SPDX header. |

### Consequences

- Add `.asc` to the bathymetry list in `README.md` "Supported File Formats" (also asked for by the plan review); mention that a sidecar `.prj` supplies the CRS.
- A helper in libgz4d is built into the app as well; rebuild the app (not only `test_libgz4d`) to check it still compiles, and confirm ctest passes with the test no longer red.
- The branch is red on purpose today (`GdalAscTest` fails); it must go green by the end of this work.
- The `GdalRaster.cpp` comment "[0]=lon and [1]=lat" reads backwards (GDAL 3, no axis-mapping strategy); keep it as a PR-description follow-up, not changed here.

### Recommendations

- Register `.asc` as `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` beside the existing aliases, in the same change as the fallback (registering without it misplaces WKT2 grids).
- Add the sidecar `.prj` fallback to `GdalRaster::Load`, used only when `GetProjectionRef()` is empty, through a libgz4d helper the gtest also calls; handle `.prj`/`.PRJ`, and report an unparsable `.prj` on `std::cerr`.
- Extend `gdal_asc.cpp` to cover the WKT2 fallback, a WKT1 `.prj` read by GDAL itself, and no `.prj` at all, keeping the lat/lon axis-order and NODATA assertions.
- Add `.asc` to README "Supported File Formats". State in the PR that the alias line is verified by review and by the owner opening a file at a screen, and list the `GdalRaster.cpp` axis comment as a follow-up.
- Keep GeoTIFF output and the WKT1 `.prj` writing out of this issue (ice_cube's).

### Actions
- [ ] The existing gtest stops at GDAL's own reading and does not run the fallback. `GdalRaster.cpp` cannot be linked into a gtest (it pulls in `MainWindow`/`Scene3D`/GL; see the plan review), so the fallback code would be untested as written. Put the sidecar read in a small function in libgz4d (for example a helper that returns a `gz4d::ogr::SpatialReference` from a raster path and the dataset's own WKT), call it from `GdalRaster::Load`, and have `gdal_asc.cpp` call that same helper: the test then passes on the WKT2 `.prj` and still asserts placement (origin at lat `[0]`, lon `[1]`) and NODATA. Add a second case with a WKT1 `.prj` showing GDAL's own path is untouched, and one with no `.prj` showing the loader still fails cleanly rather than guessing.
- [ ] Register `.asc` as `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` beside the existing aliases, in the same change as the fallback (registering without it misplaces WKT2 grids).
- [ ] Add the sidecar `.prj` fallback to `GdalRaster::Load`, used only when `GetProjectionRef()` is empty, through a libgz4d helper the gtest also calls; handle `.prj`/`.PRJ`, and report an unparsable `.prj` on `std::cerr`.
- [ ] Extend `gdal_asc.cpp` to cover the WKT2 fallback, a WKT1 `.prj` read by GDAL itself, and no `.prj` at all, keeping the lat/lon axis-order and NODATA assertions.
- [ ] Add `.asc` to README "Supported File Formats". State in the PR that the alias line is verified by review and by the owner opening a file at a screen, and list the `GdalRaster.cpp` axis comment as a follow-up.
- [ ] Keep GeoTIFF output and the WKT1 `.prj` writing out of this issue (ice_cube's).

## Checkpoint
**Status**: complete
**When**: 2026-11-15 10:17 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (the post-restart Issue Review's Actions are how to build and test the owner's restated scope; no further scope change)
**After**: issue-actions
**Decision**: proceed

The Issue Review at 4f443fe (after the restart) recommends planning the widened scope the owner chose ("Both"): a sidecar .prj fallback in GdalRaster when GDAL finds no CRS, in a small libgz4d helper that both GdalRaster::Load and the gtest call; tests for a WKT2 .prj, a WKT1 .prj and no .prj; the .asc alias and README line in the same change; std::filesystem for the path.

## Plan Authored
**Status**: complete
**When**: 2026-11-15 10:18 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-20/plan.md` at `31904cd`

Replaces the earlier plan after the owner chose "Both": a header-only libgz4d helper (gz4d/ogr/SidecarPrj.h) reads a sidecar .prj (.prj/.PRJ, std::filesystem) only when GDAL gives no CRS; GdalRaster::Load and gdal_asc.cpp (db55e60, reworked to WKT2, WKT1, no, bad and uppercase .prj cases) both call it; .asc alias and README line in the same change; ice_cube's WKT1 writing is out of scope.

## Plan Review
**Status**: complete
**When**: 2026-11-15 12:08 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: needs-work

**Issue**: #20 — Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader
**Plan**: `.agent/work-plans/issue-20/plan.md` at `31904cd`
**Branch**: `feature/issue-20`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | Four files (one new header, `GdalRaster.cpp`, `gdal_asc.cpp`, README); one PR. The ice_cube WKT1 half is kept out, as the owner set it. |
| Issue alignment | Needs work | Covers the owner's "Both" (this repo's half) and every recommendation of the 2026-11-15 issue review: helper in libgz4d called by both `Load` and the gtest, `.prj`/`.PRJ`, `std::cerr` on an unparsable `.prj`, WKT2 / WKT1 / no-`.prj` cases, README, axis comment as follow-up. But the unparsable-`.prj` behaviour cannot be built as planned on the current wrapper (finding 1). Serves: "The project roadmap goal of using GeoZui4D with file formats people use today (`docs/roadmap.md`). The project has no design document: none." Checked: fits, breaks no stated rule. |
| File targeting | Good | Right files. `gtest/CMakeLists.txt` already has the `if(GDAL_FOUND)` entry. Placement of the header is acceptable but `gz4d/gdal/` is the closer fit (finding 2). If finding 1 is resolved by changing the wrapper, `ogr/SpatialReference.h` joins the list. |
| Consequences | Good | README, app rebuild, axis-comment follow-up are listed. No other consumer of the new header. |
| Principle alignment | Good | Only what's needed: fallback only on an empty GDAL CRS. Test what breaks: the gtest runs the same helper `Load` calls and is red on the branch today. Prior art stated (GDAL's own `.prj`/`.PRJ` lookup; rejected source-grep and app-link tests). Design picture: no change, project has no design document. |
| ADR compliance | Good | 0001: header-only, no CMake change. 0002: `std::filesystem` for the sidecar path. 0003: SPDX header on the new file. |
| ROS conventions | N/A | Not a ROS project. |

### Findings

1. **[Issue alignment] Must-fix** — The plan's helper cannot detect a `.prj` that does not parse. `gz4d::ogr::SpatialReference::SetFromUserInput` (`libgz4d/cxx/include/gz4d/ogr/SpatialReference.h:82-85`) returns `void`, drops the `OGRErr`, and calls `init()` first, so after garbage input the wrapper is non-empty (`operator bool` true) with an empty `OGRSpatialReference` inside. The raw-pointer constructors are private (friends only), so the helper cannot build the result from a local `OGRSpatialReference` either. As written, `UnparsablePrjReported` would fail and `Load` would treat the bad CRS as a valid projected one (`!IsGeographic()` is true for an empty SRS) and run `Projection` on it. The plan must name the mechanism: either (a) validate the text first with a local `OGRSpatialReference::SetFromUserInput` and call the wrapper only on `OGRERR_NONE`, leaving `SpatialReference.h` untouched; or (b) change the wrapper's `SetFromUserInput` to return `OGRErr` (source-compatible for the seven existing callers, which ignore the result: `Scene3D.cpp:1559`, `Object.cpp:326,2104`, `gzCommandInterface.cpp:1685`, `GdalRaster.cpp:82`, `geo/Projection.h:40`) and `reset()` on failure. (a) is smaller and keeps the change local; (b) adds `SpatialReference.h` to the file list. The same question applies to a non-empty `GetProjectionRef()` string, though GDAL produced it, so a failure there is unlikely.
2. **[File targeting] Suggestion** — Placement asked by the planner. `gz4d/ogr/` holds the thin RAII wrappers over OGR types (`SpatialReference`, `Geometry`, `Layer`, `DataSource`, `CoordinateTransformation`), all PascalCase class headers; it has no free functions and nothing that reads files beside a raster. `gz4d/gdal/` (`saveGeotiff.h`: `inline` free function `gz4d::gdal::saveGeotiff`, plus `Dataset.h`, `RasterBand.h`) is where raster-dataset helpers already live, and it already depends on the ogr headers. A sidecar lookup for a raster dataset reads more naturally as `gz4d/gdal/sidecarPrj.h` with `gz4d::gdal::spatialReferenceForRaster(...)`, returning a `gz4d::ogr::SpatialReference`. The `ogr/` placement is not wrong (it compiles and both callers have GDAL), so this is the planner's call; whichever is chosen, define the function `inline` in-namespace and include `<filesystem>`, `<fstream>`, `<iostream>` explicitly (libgz4d headers already use `std::cerr`, e.g. `Angle.h`, `Kalman.h`).
3. **[Issue alignment] Suggestion** — The issue review asked for a no-`.prj` case "showing the loader still fails cleanly rather than guessing"; the plan keeps today's behaviour (empty CRS, `Load` proceeds and treats the grid coordinates as geographic). That is consistent with "Only what's needed" and with GeoTIFFs that are geographic but carry no CRS, but say in the PR that a metre `.asc` with no `.prj` is still misplaced as degrees, so the owner knows the gap is deliberate.
4. **[Principle alignment] Suggestion** — In `UnparsablePrjReported`, GDAL/PROJ will also print its own `CPLError` text to stderr; assert that the captured text contains the `.prj` filename (the helper's message), not merely that stderr is non-empty. For `UppercasePrj`, decide the case-insensitive-FS skip by probing (write `x.PRJ`, check `exists(x.prj)`), not by platform macro.
5. **[Principle alignment] Confirmed (planner's question)** — The `.asc` alias is checked by code review and by the owner opening a file at a screen, not by ctest. Agreed: `REGISTER_ALIAS` is a one-line static registration in `GdalRaster.cpp` beside the `tif`/`tiff`/`vrt` lines, the app has no objects library a test could link (`GdalRaster.cpp` pulls in `MainWindow`, `Scene3D`, GL), and a source-grep check would test the text, not the registration. The real risk (CRS placement) is covered by the gtest through the shared helper. State this in the PR, as planned.

### Summary

The plan carries the owner's "Both" decision into this repo correctly and puts the fallback where the gtest can run it. One mechanism gap blocks it: the existing `SpatialReference` wrapper hides parse errors, so the planned unparsable-`.prj` report and empty result cannot work without naming how the error is detected. Once that is settled, it is ready.

### Recommended Actions

- [ ] Name how the helper detects a failed parse (validate with a local `OGRSpatialReference` first, or have the wrapper's `SetFromUserInput` return `OGRErr` and add `SpatialReference.h` to Files to Change).
- [ ] Decide `gz4d/ogr/SidecarPrj.h` vs `gz4d/gdal/` (suggestion; either is acceptable).
- [ ] Note in the PR that a no-`.prj` metre grid is still placed as degrees (deliberate).
- [ ] Tighten the stderr and `.PRJ` test details as in finding 4.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 12:08 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (plan review needs-work with one must-fix; first host revise at this plan)
**After**: plan
**Decision**: revise

The Plan Review at ec2062d found that gz4d::ogr::SpatialReference::SetFromUserInput returns void and leaves a wrapper that tests true after garbage input, so the helper cannot detect an unparsable .prj. Host choices for the revision (mechanism, listed for the owner's veto): fix (a), parse the .prj text with a local OGRSpatialReference first and hand it to the wrapper only on success, leaving SpatialReference.h unchanged; place the helper as gz4d::gdal::spatialReferenceForRaster in gz4d/gdal/ beside saveGeotiff.h; keep today's behaviour when there is no .prj and say in the PR that the gap is deliberate; in the bad-.prj test check that stderr names the .prj file; detect a case-insensitive file system by writing a file and checking for it, not by platform macro.

## Plan Authored
**Status**: complete
**When**: 2026-11-15 12:08 -05:00
**By**: Claude Code Agent (claude-sonnet-5-5)
**Plan**: `.agent/work-plans/issue-20/plan.md` at `6d3d34e`

Revision after the ec2062d review: the helper gz4d::gdal::spatialReferenceForRaster (gz4d/gdal/sidecarPrj.h) parses the .prj text with a local OGRSpatialReference first and builds the gz4d::ogr::SpatialReference only on OGRERR_NONE (SpatialReference.h unchanged); a bad .prj reports on stderr naming the file; no .prj keeps today's behaviour (deliberate, to be said in the PR); tests check the stderr file name and probe for a case-insensitive file system.

## Plan Review
**Status**: complete
**When**: 2026-11-15 12:09 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: ready

**Issue**: #20 — Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader
**Plan**: `.agent/work-plans/issue-20/plan.md` at `6d3d34e`
**Branch**: `feature/issue-20`

### Evaluation

| Dimension | Verdict | Notes |
|---|---|---|
| Scope | Good | Four files (new `gz4d/gdal/sidecarPrj.h`, `GdalRaster.cpp`, `gdal_asc.cpp`, README); one PR. The ice_cube WKT1 half stays out, as the owner set it ("Both"; ice_cube's half done there). |
| Issue alignment | Good | The ec2062d must-fix is resolved by option (a): parse with a local `OGRSpatialReference::SetFromUserInput`, check `OGRERR_NONE`, and only then build the wrapper from the same text. Checked against `SpatialReference.h`: `SetFromUserInput` returns `void` (lines 82-85) and the raw constructors are private, so (a) is the only route that leaves the header unchanged, and a default wrapper gives `operator bool` false as the plan says. The non-empty-GDAL-WKT path uses the same check. Every issue-review recommendation is carried (helper shared by `Load` and the gtest, `.prj`/`.PRJ`, `std::cerr` naming the file, WKT2/WKT1/no/bad/uppercase cases, README, axis comment as follow-up). Serves: "The project roadmap goal of using GeoZui4D with file formats people use today (`docs/roadmap.md`). The project has no design document: none." Checked: fits, breaks no stated rule. |
| File targeting | Good | `gz4d/gdal/` beside `saveGeotiff.h` (inline free function, `gz4d::gdal`) matches existing placement; explicit `<filesystem>`/`<fstream>`/`<iostream>`/`<sstream>` includes listed. `gtest/CMakeLists.txt` already has the `if(GDAL_FOUND)` entry; `SpatialReference.h` correctly not listed. |
| Consequences | Good | README, app rebuild, axis-comment follow-up. `Load` is called right after creation (`Scene3D.cpp:655`, `ObjectCollection.cpp:458`), so no CRS is normally set on the object before `Load`; see finding 1 for keeping that untouched anyway. |
| Principle alignment | Good | Only what's needed: fallback only when GDAL gives no CRS; no wrapper change. Test what breaks: the gtest runs the helper `Load` calls; branch goes from red to green. Prior art stated (GDAL's `.prj`/`.PRJ` lookup, `saveGeotiff.h` placement; rejected wrapper change, source-grep, app-link test). Design picture: no change. The no-`.prj` gap is stated as deliberate (ec2062d finding 3). Test details from ec2062d finding 4 adopted (stderr names the file; case-insensitive FS probed, no platform macro). |
| ADR compliance | Good | 0001: header-only, no CMake change. 0002: `replace_extension`/`exists` via `std::filesystem`. 0003: SPDX header on the new file. |
| ROS conventions | N/A | Not a ROS project. |

Host mechanism choices checked: local-parse-first with `SpatialReference.h` unchanged (sound, smallest); `gz4d::gdal::spatialReferenceForRaster` in `gz4d/gdal/` (fits the existing layout); no `.prj` keeps today's behaviour, stated as a deliberate gap (consistent with "Only what's needed"). No objection to any.

### Findings

1. **[Consequences] Suggestion** — Today `Load` leaves `spatialReference` untouched when GDAL gives no CRS. "Replace lines 79-82 with a call to the helper" would, if written as an unconditional `spatialReference = spatialReferenceForRaster(...)`, overwrite any CRS already on the object with an empty one. Normal loading creates the object and calls `Load` at once, so the risk is small, but assigning only when the result is non-empty (`if(auto sr = ...; sr) spatialReference = sr;`) keeps today's semantics exactly at no cost.
2. **[Principle alignment] Suggestion** — `UppercasePrj`: write WKT2 into `x.PRJ`. GDAL's AAIGrid driver also looks for `.PRJ`, so a WKT1 `.PRJ` would be read by GDAL itself and the helper's `.PRJ` lookup would never run; WKT2 makes the test exercise the fallback path it is named for (assert `GetProjectionRef()` empty as in `Wkt2PrjUsesFallback`, with the same tolerance for a later GDAL).
3. **[Principle alignment] Suggestion** — `Wkt1PrjReadByGdal`: comparing the helper's result with a wrapper built from `GetProjectionRef()` is close to tautological (the helper passes that same text through). Compare instead with the UTM SRS the test built to write the `.prj` (`IsSame`), which checks the round trip. "No fallback read" is not directly observable; asserting nothing on captured stderr is enough.
4. **[Principle alignment] Suggestion (optional hardening)** — `OGRSpatialReference::SetFromUserInput` also treats its argument as a file name, URL or `EPSG:` code. For `.prj` contents that is harmless in practice; GDAL 3.x's two-argument overload with `ALLOW_NETWORK_ACCESS=NO`/`ALLOW_FILE_ACCESS=NO` would close it for the local parse, but the wrapper's one-argument call would still differ. Not worth widening scope; mention only if the implementer touches it.
5. **[Issue alignment] Confirmed (planner's question)** — The `.asc` alias checked by review and by the owner at a screen, not by ctest, remains acceptable (as in ec2062d finding 5).

### Summary

The revision resolves the previous must-fix with a mechanism that works on the existing wrapper unchanged, and adopts all four earlier suggestions. The remaining points are small test and assignment details the implementer can take in stride. Ready for implementation.

### Recommended Actions

- [ ] In `Load`, assign the helper's result only when it is non-empty (finding 1).
- [ ] Use WKT2 content in the `UppercasePrj` test so the fallback's `.PRJ` lookup is what runs (finding 2).
- [ ] In `Wkt1PrjReadByGdal`, compare against the SRS the test built, not GDAL's own string (finding 3).

## Checkpoint
**Status**: complete
**When**: 2026-11-15 12:10 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (plan review verdict ready, no must-fix)
**After**: plan
**Decision**: proceed

The Plan Review at 5fea17f is ready. Its suggestions go to the implement pass: in GdalRaster::Load assign the helper's result only when it is non-empty; write WKT2 into the UppercasePrj test's .PRJ; compare Wkt1PrjReadByGdal's result with the CRS the test built; and, since a .prj is untrusted input, use the options form of OGRSpatialReference::SetFromUserInput for the local parse with network and file access turned off (host choice: cheap, and it keeps a .prj from naming a URL or file to fetch).

## Implementation
**Status**: complete
**When**: 2026-11-15 12:24 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Branch**: feature/issue-20 at `f15d118`

- `829c004`: new header-only `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h`, `gz4d::gdal::spatialReferenceForRaster(datasetWkt, rasterPath)`. Uses GDAL's CRS when non-empty; otherwise reads `<stem>.prj` then `<stem>.PRJ` (`std::filesystem`). Parses with a local `OGRSpatialReference::SetFromUserInput(text, {ALLOW_NETWORK_ACCESS=NO, ALLOW_FILE_ACCESS=NO})` (two-argument form and both options present in GDAL 3.8.4's `ogr_spatialref.h` / libgdal) and builds the wrapper only on `OGRERR_NONE`; an unparsable `.prj` is reported on `std::cerr` naming the file and gives an empty reference. `SpatialReference.h` unchanged. `gdal_asc.cpp` reworked into five cases calling the helper.
- `f15d118`: `GdalRaster::Load` calls the helper and assigns `spatialReference` only when the result is non-empty; `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc")`; README lists `.asc`.
- Plan-review suggestions taken: non-empty-only assignment; WKT2 in the `UppercasePrj` `.PRJ`; `Wkt1PrjReadByGdal` compares with the WGS 84 / UTM 23N CRS the test built (`IsSame`), not GDAL's string.
- Verification (scratch build, `-DBUILD_GTESTS=ON`, `-j6`): `test_libgz4d` 28/28 passed; GdalAscTest: Wkt2PrjUsesFallback, Wkt1PrjReadByGdal, NoPrjHandledCleanly, UnparsablePrjReported, UppercasePrj all passed (not skipped; GDAL 3.8.4 gave no CRS for either WKT2 sidecar, so the fallback ran). ctest 1/1 passed. Full app (`GeoZui4D`) built, no new warnings from the changed files. Scratch dirs removed.
- For the PR: the `.asc` alias is checked by review and by the owner opening a file, not by ctest; a metre `.asc` with no `.prj` is still placed as degrees (deliberate); the `GdalRaster.cpp` "[0]=lon and [1]=lat" comment is backwards under GDAL 3 and left unchanged as a follow-up.

## Local Review (Pre-Push)
**Status**: complete
**When**: 2026-11-15 12:37 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Verdict**: approved

**Branch**: feature/issue-20 at `8931f4f`
**Base**: main
**Depth**: Deep (reason: 849 changed lines incl. plan/progress; ~400 lines of code across 5 files)
**Must-fix**: 0 | **Suggestions**: 6
**Round**: 1 | **Ship**: recommended — no must-fix findings; remaining suggestions can be applied or tracked

Merged branch (with issue 16's GoogleTest fetch) built with -DBUILD_GTESTS=ON: full build incl. GeoZui4D app OK, ctest 1/1 passed (28 tests, 5 GdalAscTest), no compiler warnings in changed files. Also built with -DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON (fetched GoogleTest v1.14.0 + gdal_asc.cpp): test_libgz4d 28/28 passed. Static analysis: cppcheck and clang-tidy not installed (C++ files not linted). Cross-model: gemini and codex completed (suggestions only, folded in below). Claude adversarial: no must-fix.

### Findings
- [ ] (suggestion) Bound the sidecar size before reading the whole `.prj` into memory (codex + gemini) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h:73`
- [ ] (suggestion) Check that the `.prj` opened/read; an unreadable file is reported as "could not parse" (gemini + Claude adversarial) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h:71`
- [ ] (suggestion) Two-argument `OGRSpatialReference::SetFromUserInput(text, options)` needs GDAL 3.x; no minimum GDAL version is stated or guarded (`find_package(GDAL)` has none) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h:34`
- [ ] (suggestion) Agent guide says the gtest suite has "nine sources"; gdal_asc.cpp is now a tenth when GDAL is found — `.agents/README.md:148`
- [ ] (suggestion) `M_PI` is not standard C++ (MSVC needs `_USE_MATH_DEFINES`); use a local constexpr — `libgz4d/cxx/gtest/gdal_asc.cpp:163`
- [ ] (suggestion) The `.prj` fallback also applies to CRS-less `.tif`/`.tiff`/`.vrt`, not only `.asc`; say so in the comment or the PR — `GeoZui4D/main/objects/GdalRaster.cpp:81`

## Checkpoint
**Status**: complete
**When**: 2026-11-15 12:38 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: host (pre-push round 1 approved, ship recommended, no must-fix; the standing rule takes a second round only after a must-fix)
**After**: publish
**Decision**: publish

The Local Review (Pre-Push) at 60d27d5 approved with six suggestions, named in the PR as follow-ups: bound the .prj read; say "could not read" for an unreadable .prj; the options form of SetFromUserInput needs GDAL 3 (stated in the PR, not checked in CMake: GDAL 3 is in every supported Ubuntu); the "nine sources" count in .agents/README.md; M_PI on MSVC; the fallback also serves .tif/.tiff/.vrt files with no CRS.

## Integrated Review
**Status**: complete
**When**: 2026-11-15 12:39 -05:00
**By**: Claude Code Agent (claude-opus-5-5)

**PR**: #23 at `5f9c990`
**Sources**: 2 (Local Review (Pre-Push) @ `8931f4f` (covers 5f9c990 via bookkeeping; recorded at 60d27d5), GitHub fetch @ `5f9c990`: 0 reviews, 0 inline comments, 0 conversation comments, 0 check-runs)
**Cross-source confirmations**: 0
**CI**: all-pass (vacuous: the repository has no CI workflow, so 0 check-runs exist at the head; issue 12 adds CI)

All six pre-push suggestions re-checked against the code at 5f9c990 and still apply. None is must-fix; the publish Checkpoint names them in the PR as known follow-ups.

### Findings
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) Bound the sidecar size before reading the whole `.prj` into memory (codex + gemini) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h`
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) Check that the `.prj` opened/read; an unreadable file is reported as "could not parse" (gemini + Claude adversarial) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h`
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) Two-argument `OGRSpatialReference::SetFromUserInput(text, options)` needs GDAL 3.x; no minimum GDAL version is stated or guarded (`find_package(GDAL)` has none) — `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h`
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) Agent guide says the gtest suite has "nine sources"; gdal_asc.cpp is now a tenth when GDAL is found — `.agents/README.md`
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) `M_PI` is not standard C++ (MSVC needs `_USE_MATH_DEFINES`); use a local constexpr — `libgz4d/cxx/gtest/gdal_asc.cpp`
- [ ] (suggestion, Local Review (Pre-Push) @ `8931f4f`) The `.prj` fallback also applies to CRS-less `.tif`/`.tiff`/`.vrt`, not only `.asc`; say so in the comment or the PR — `GeoZui4D/main/objects/GdalRaster.cpp`

### False positives
- None.

## Checkpoint
**Status**: complete
**When**: 2026-11-15 15:39 -05:00
**By**: Claude Code Agent (claude-opus-5-5)
**Decided-by**: owner
**After**: findings
**Decision**: merge

Asked "Merge PR #23 with F1–F6 as follow-ups, or fix them first?" (the six pre-push suggestions), the owner answered: "Merge, F1–F6 as follow-ups". Asked "If you merge in Q1, use --no-wait for PR #23?" (the repository has no CI), he answered: "Yes, --no-wait".
