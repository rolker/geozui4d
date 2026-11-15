# Plan: Open ESRI ASCII grids (.asc) as surfaces through the GDAL raster loader

## Issue

https://github.com/rolker/geozui4d/issues/20

## Context

`GdalRaster` (`GeoZui4D/main/objects/GdalRaster.cpp:20-22`) is registered for `tif`, `tiff` and `vrt` only, so an `.asc` grid falls through to the texture loader (`Scene3D.cpp:645-680`). `Load()` already places a grid from its CRS (`GetProjectionRef` -> `SetFromUserInput` -> `gz4d::geo::Projection`) and turns NODATA into NaN (line 154). The one unknown is whether GDAL 3.8.4's AAIGrid driver reads the WKT2 `.prj` ice_cube writes (`LocalFrame.wkt2()`: an ellipsoidal orthographic, EPSG method 9840, about a local origin). If it does not, `spatialReference` stays empty and the grid is placed as if metres were degrees.

Owner decision (2026-11-15): do option 1 (register `.asc`); GeoTIFF output is ice_cube's business and out of scope.

## Approach

1. **Write the test first** in `libgz4d/cxx/gtest/gdal_asc.cpp`, with a made-up origin (lat 12.3456, lon -45.6789, nothing from ice_cube data). The test writes a 4x3 `.asc` (`NODATA_value -9999`, one cell set to -9999, float values, `xllcorner`/`yllcorner` in metres about the origin) and a `.prj` holding a WKT2 string copied in form from `LocalFrame.wkt2()` (same PROJCRS, METHOD "Orthographic" ID EPSG 9840, false easting/northing 0, east/north axes, USAGE/BBOX). The files go in a per-test temp directory under `std::filesystem::temp_directory_path()` and are removed afterwards.
2. **Assertions**, using the same calls `GdalRaster::Load` makes:
   - `GDALOpen` succeeds; `GetProjectionRef()` is non-empty. This is the assertion that fails first if the AAIGrid driver ignores the WKT2 `.prj`. Per the host's instruction, if it fails the work **stops and reports**; no sidecar `.prj` fallback is added (owner's call).
   - `SetFromUserInput(projWkt)` gives a projected, non-geographic CRS.
   - A `gz4d::geo::Projection(srs, WGS84)` of the projected coordinate (0, 0) gives the origin lat/lon to within 1e-9 deg (the ortho projection about the origin is exact there). A second point, 100 m east, is checked against the ENU expectation at a looser tolerance.
   - The NODATA cell read through `RasterIO(GDT_Float32)` equals `-9999`, and the same comparison `GdalRaster` makes (`z == (float)noDataValue`, `GetNoDataValue` flagged) maps it to NaN, while the other cells stay finite. The mapping is two lines in `Load()` that cannot be called without the app, so the test repeats the comparison as written; this is noted in a comment.
3. **Register the alias**: add `REGISTER_ALIAS(GdalRasterAsc, GdalRaster, "asc");` after the `vrt` line in `GdalRaster.cpp`.
4. **Registration check.** `GdalRaster` is app code in the `GeoZui4D` executable (the registry in `engine/Object.cpp` is app-side too), so a libgz4d gtest cannot link it and cannot ask the registry whether `asc` maps to `GdalRaster`. The gdal test lives in libgz4d's gtest suite because that is the only test harness in the repo (`BUILD_GTESTS`, `ctest` via `scripts/build.sh`) and GDAL and `gz4d::geo::Projection` are reachable from there. For the registration, add a ctest in the same gtest directory that is a CMake script (`cmake -P`) reading `GeoZui4D/main/objects/GdalRaster.cpp` and failing unless it contains a `REGISTER_ALIAS(..., GdalRaster, "asc")` line. It is a source check, not a runtime one; the runtime check (opening an `.asc` in the app and seeing a surface) is left for the owner at a screen. This weakness is stated in the PR.
5. **Wire the test** into `libgz4d/cxx/gtest/CMakeLists.txt`: add `gdal_asc.cpp` to `TEST_SOURCES` only `if(GDAL_FOUND)` (libgz4d's CMake finds GDAL optionally and already adds it to `LIBS`), and add the registration `add_test`. Confirm that `scripts/build.sh` plus ctest picks both up.
6. **Verify**: build with `-DBUILD_GTESTS=ON`; run ctest. Check the test fails before the `.prj` is present (temporarily omitting it) so the guard is shown to bite. Build the app to confirm `GdalRaster.cpp` compiles.

## Files to Change

| File | Change |
|------|--------|
| `GeoZui4D/main/objects/GdalRaster.cpp` | Add the `asc` alias line |
| `libgz4d/cxx/gtest/gdal_asc.cpp` | New test (SPDX header), synthetic grid + WKT2 `.prj` |
| `libgz4d/cxx/gtest/CMakeLists.txt` | Add the test source when GDAL is found; add the registration source-check ctest |
| `libgz4d/cxx/gtest/check_asc_registered.cmake` | New cmake -P script for the source check |

## Prior Art

`GdalRaster::Load` itself is the reference: the test copies its GDAL and Projection calls rather than inventing a path. The existing gtest suite (`libgz4d/cxx/gtest/`) gives the harness and SPDX header pattern. Rejected: a stand-alone program (not run by ctest); a sidecar `.prj` reader in `GdalRaster` (widens the issue, owner's call); linking the app into a test (the app is an executable with Tcl/GL dependencies).

## Serves

The project roadmap goal of using GeoZui4D with file formats people use today (`docs/roadmap.md`). The project has no design document: none.

## Design Picture

No change to the design picture. It adds an alias to an existing loader.

## Principles Self-Check

| Principle | Consideration |
|---|---|
| Only what's needed | One registration line plus the check of the acceptance; no fallback, no GeoTIFF work |
| Test what you change | The test runs the real GDAL and Projection calls; fails first if the `.prj` is not read |
| Private data stays private | Made-up origin and values, generated in the test; nothing from ice_cube data |

## ADR Compliance

| ADR | Triggered | How addressed |
|---|---|---|
| 0001 CMake 3.14, C++17 | Watch | Only the sub-CMakeLists in `gtest/` changes; top-level `CMakeLists.txt` untouched; cmake -P script is plain 3.14 |
| 0002 std::filesystem | Yes | Test uses `std::filesystem` for the temp directory |
| 0003 Apache 2.0 | Yes | SPDX header on the new files |

## Consequences

| If we change... | Also update... | Included in plan? |
|---|---|---|
| gtest sources | README test steps / `scripts/build.sh` already run ctest | Yes, confirmed in step 5 |
| Supported surface extensions | No document lists them (searched `docs/`) | n/a |

## Open Questions

- If GDAL 3.8.4 does not read the WKT2 `.prj`, the plan stops there and reports; whether to add a sidecar `.prj` fallback is the owner's decision.
- The runtime registration check is a source check only; an app-level check needs a person at a screen. Acceptable?

## Estimated Scope

Single PR.
