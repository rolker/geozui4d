# Plan: Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader

## Issue

https://github.com/rolker/geozui4d/issues/20

## Context

`GdalRaster` (`GeoZui4D/main/objects/GdalRaster.cpp:20-22`) is registered for `tif`, `tiff` and `vrt` only, so an `.asc` falls through to the texture loader. `Load()` takes the CRS only from `ds->GetProjectionRef()` (lines 79-82); when empty, `spatialReference` stays empty and line 93 places a metre grid as if in degrees. GDAL 3.8.4's AAIGrid driver reads a `.prj` as ESRI WKT1 only, so the WKT2 `.prj` ice_cube writes leaves the grid with no CRS (found on this branch).

Owner decisions (2026-11-15): register `.asc` (option 1); on "How should the CRS reach GeoZui?": "Both" (ice_cube writes WKT1 in `.prj` AND GeoZui reads a WKT2 `.prj` itself). This plan replaces the earlier one (alias plus stop-and-report). The ice_cube half is done in ice_cube and is not part of this issue.

Existing branch commits: `db55e60` adds `libgz4d/cxx/gtest/gdal_asc.cpp` (synthetic 4x3 grid, made-up origin 12.3456/-45.6789, WKT2 `.prj`, test `ReadsWkt2PrjPlacementAndNoData`) and its `if(GDAL_FOUND)` entry in `gtest/CMakeLists.txt`; it fails on purpose at `GetProjectionRef`. The other commits (`6cd70dd`, `602e0fb`, `4f443fe`, `90d265c`) are progress entries only. This plan builds on `db55e60`: it keeps the test file and its CMake wiring and changes where the CRS comes from.

## Approach

1. **Helper in libgz4d** (header-only, like most of libgz4d; no CMake change): new `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h` (SPDX header, beside `saveGeotiff.h`; explicit `<filesystem>`, `<fstream>`, `<iostream>`, `<sstream>` includes) with `inline gz4d::ogr::SpatialReference gz4d::gdal::spatialReferenceForRaster(const std::string &datasetWkt, const std::filesystem::path &rasterPath)`.
   - Parse check: the wrapper's `SetFromUserInput` returns `void` and leaves a non-empty wrapper holding an empty SRS after garbage, and its raw-reference constructors are private. So the helper parses the text first with a local `OGRSpatialReference::SetFromUserInput` (accepts WKT1 and WKT2) and checks for `OGRERR_NONE`; only on success does it call `SetFromUserInput(text)` on a fresh wrapper with the same text (lossless; the text is parsed twice, which is cheap). `SpatialReference.h` stays unchanged. Failure returns a default (empty) wrapper, so `operator bool` is false.
   - Non-empty `datasetWkt`: same parse check, then the wrapper; GDAL's own result is never overridden. A parse failure here (unlikely, GDAL produced it) returns empty and reports on `std::cerr` as below.
   - Empty: try `<stem>.prj` then `<stem>.PRJ` (`replace_extension`, `std::filesystem`, ADR-0002); read the first that exists.
   - A `.prj` that exists but does not parse: message on `std::cerr` naming the `.prj` file; return an empty reference. No `.prj`: return empty, silently (as today). This gap is deliberate: a metre `.asc` with no `.prj` is still placed as if in degrees, as any CRS-less raster is today; the PR says so.
2. **Use it in `GdalRaster::Load`**: replace lines 79-82 with a call to the helper using `ds->GetProjectionRef()` and `GetFilename()`. Include the header. The rest of `Load` is unchanged (an empty CRS behaves as before).
3. **Register the alias**: `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` after the `vrt` line, in the same change as the fallback.
4. **Rework `gdal_asc.cpp`** so the test goes green and covers the fallback: a fixture helper writes the grid and an optional `.prj` (parameter: WKT2, WKT1, none, garbage). Each test opens with GDAL as `Load` does, calls the helper with `GetProjectionRef()` and the path, then makes the same Projection/NODATA checks.
   - `Wkt2PrjUsesFallback`: GDAL's `GetProjectionRef()` is empty (documents the reason for the helper; if a later GDAL reads WKT2 this assertion is relaxed to "either way the helper result is right"); helper result is projected; (0,0) maps to lat `[0]` 12.3456, lon `[1]` -45.6789 within 1e-9; 100 m east checked as now; NODATA cell becomes NaN, others finite.
   - `Wkt1PrjReadByGdal`: an ESRI-WKT1 `.prj` (e.g. a UTM zone written by `exportToWkt`/`morphToESRI` of a WGS84 UTM SRS built in the test) gives non-empty `GetProjectionRef()`; helper returns a projected CRS equal to GDAL's (`IsSame`); no fallback read.
   - `NoPrjHandledCleanly`: no `.prj`; helper returns an empty reference, no throw, nothing on stderr.
   - `UnparsablePrjReported`: garbage `.prj` gives an empty reference (`operator bool` false); captured stderr (`testing::internal::CaptureStderr`) contains the `.prj` file name (GDAL/PROJ may add its own text, so non-empty is not enough).
   - `UppercasePrj`: `.PRJ` is found (the test writes `x.PRJ` and checks whether `x.prj` exists; if so the file system is case-insensitive and the test is skipped; no platform macro).
   Keep the lat-in-`[0]` axis assertion and the NODATA assertion. Synthetic data only; temp dir via `std::filesystem`, removed in `TearDown`.
5. **README**: add `.asc` (ESRI ASCII grid, CRS from a sidecar `.prj`) to the bathymetry list under "Supported File Formats".
6. **Verify**: build with `-DBUILD_GTESTS=ON` and run ctest (the branch goes green); build the full app so `GdalRaster.cpp` compiles with the header. The alias line and runtime behaviour of the app are checked by review and by the owner opening an `.asc` at a screen; state that in the PR, and list the `GdalRaster.cpp` "[0]=lon, [1]=lat" comment (backwards under GDAL 3) as a follow-up, not changed here.

## Files to Change

| File | Change |
|------|--------|
| `libgz4d/cxx/include/gz4d/gdal/sidecarPrj.h` | New header-only helper `gz4d::gdal::spatialReferenceForRaster` |
| `GeoZui4D/main/objects/GdalRaster.cpp` | Call helper at lines 79-82; add `asc` alias |
| `libgz4d/cxx/gtest/gdal_asc.cpp` | Rework: WKT2 fallback, WKT1, no `.prj`, bad `.prj`, `.PRJ` cases |
| `README.md` | `.asc` in Supported File Formats |

`libgz4d/cxx/gtest/CMakeLists.txt` already has the GDAL-guarded entry; no change expected.

## Prior Art

`GdalRaster::Load` and `db55e60` are the reference for the GDAL/Projection calls. Placement follows `saveGeotiff.h` (free `inline` raster helper in `gz4d/gdal/`; `gz4d/ogr/` holds class wrappers only). Changing the wrapper's `SetFromUserInput` to return `OGRErr` was rejected as larger (seven callers, one more file). GDAL's AAIGrid driver looks for `.prj` and `.PRJ` (taken: both spellings) but parses ESRI WKT1 only (the gap). Rejected: converting the WKT2 to WKT1 in GeoZui (ice_cube writes WKT1 already; the fallback only needs `SetFromUserInput`); a CMake source-check for the alias (the earlier plan's idea; the owner views the app, the line is one-liner reviewed); a runtime link of the app into a test (GL/Tcl dependencies).

## Serves

The project roadmap goal of using GeoZui4D with file formats people use today (`docs/roadmap.md`). The project has no design document: none.

## Design Picture

No change to the design picture; an alias and a CRS fallback inside an existing loader.

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Only what's needed | Fallback only when GDAL gives no CRS; no CRS rewriting; no GeoTIFF or ice_cube work |
| Test what you change | The fallback lives in libgz4d so the gtest runs the same code as `Load`; all four `.prj` situations covered |
| Private data stays private | Synthetic grid and made-up origin; nothing from ice_cube data |
| Put each thing at the level it applies to | The WKT2 reading helps any WKT2 `.prj`; WKT1 writing stays in ice_cube |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| 0001 CMake 3.14, C++17 | Watch | Header-only helper; no CMake change; top-level `CMakeLists.txt` untouched |
| 0002 std::filesystem | Yes | Path built and probed with `std::filesystem` |
| 0003 Apache 2.0 | Yes | SPDX header on the new header |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| Supported surface extensions | README "Supported File Formats" | Yes, step 5 |
| A libgz4d header | App rebuild to confirm it compiles | Yes, step 6 |
| `Load` CRS source | Axis comment in `Load` (backwards) | No, PR follow-up |

## Open Questions

- None blocking. Host mechanism choices listed for the owner's veto: local-parse-first, `gz4d/gdal/` placement, no-`.prj` unchanged. If the installed GDAL ever reads the WKT2 `.prj` itself, the helper simply does not fall back; the test tolerates both.
- The alias registration is checked by review and by the owner at a screen, not by ctest. Acceptable?

## Estimated Scope

Single PR.
