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
