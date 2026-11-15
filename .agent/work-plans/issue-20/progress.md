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
