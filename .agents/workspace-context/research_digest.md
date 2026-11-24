# Research Digest: GeoZui4D

<!-- Last full refresh: never -->
<!-- Run /research --refresh periodically to re-survey stale entries -->

How to read the entries. Each finding carries one of three marks:

- **[V]** verified here by running something; the entry says what was run.
- **[S]** stated by the source linked beside it (a primary source unless the
  entry says otherwise); not re-derived here.
- **[I]** an inference of this digest's author, not stated by any source.

The machine the [V] checks ran on (2026-11-24): PROJ 9.7.1 (`proj-bin`,
`proj-data` 9.7.1-1; database EPSG v12.029 of 2025-10-03, read with
`sqlite3 /usr/share/proj/proj.db "select * from metadata"`), GDAL 3.12.2
Python bindings. Every numerical check uses one **made-up** point, latitude
43.0, longitude -71.0, ellipsoidal height 0 (ECEF 1521039.2168 -4417418.6389
4327501.7591 on GRS80); it is no real site.

## PROJ for time-dependent transformations (issue #28, question 1)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: [PROJ helmert](https://proj.org/en/stable/operations/transformations/helmert.html), [PROJ deformation](https://proj.org/en/stable/operations/transformations/deformation.html), [PROJ defmodel](https://proj.org/en/stable/operations/transformations/defmodel.html), [PROJ network](https://proj.org/en/stable/usage/network.html), [PROJ C API](https://proj.org/en/stable/development/reference/functions.html), [PROJ news](https://proj.org/en/stable/news.html), [how operations are chosen](https://proj.org/en/stable/operations/operations_computation.html)

Key takeaways:
- **The ITRF to NAD83(2011) transformations need no grid and work offline.**
  [V] `projinfo -s EPSG:9988 -t EPSG:6317 -o PROJ` (ITRF2020), `-s EPSG:7789`
  (ITRF2014) and `-s EPSG:5332` (ITRF2008) each return exactly one candidate:
  EPSG:10334, EPSG:8970 and EPSG:7807, each a single `+proj=helmert` step with
  rates, `+convention=coordinate_frame`, marked "time-dependent operation",
  accuracy 0.0 m as EPSG records it. ITRF2020 to ITRF2014 and to ITRF2008 are
  also in the init file `/usr/share/proj/ITRF2020` (13 older ITRFs).
- **`+proj=helmert`** takes 3, 7 or 14 parameters; rotations in arc seconds,
  scale in ppm, rates per year, `+t_epoch` in decimal years; `+convention`
  (`position_vector` or `coordinate_frame`) is required as soon as a rotation
  is given, and the two differ by the sign of the rotations. [S] helmert page.
- **A coordinate without a time is transformed silently at the operation's
  own reference epoch.** [V] `echo "X Y Z" | cs2cs EPSG:9988 EPSG:6317` gives
  the same result as the input with time 2010.0 (EPSG:10334 has
  `t_epoch=2010`); through EPSG:7807 (`t_epoch=1997`) it equals time 1997.0.
  At the made-up point the 2010.0 and 2026.9 answers differ by 0.29 m east
  and 0.12 m north. PROJ 9.7.1 gives no warning. [S] PROJ 9.9.0 adds
  `proj_crs_is_dynamic()` and a `cs2cs` warning for this case;
  `proj_coordoperation_requires_per_coordinate_input_time()` exists since 9.5
  (news, C API).
- **Epochs in the API**: 4D coordinates (time as the fourth value, decimal
  year) since PROJ 5; `CoordinateMetadata` (a CRS plus a coordinate epoch) and
  WKT2 `COORDINATEMETADATA[]` since 9.2; `proj_coordinate_metadata_create()`
  and point motion operations (a change of coordinate epoch) since 9.4;
  `ONLY_BEST` since 9.2. [S] news, C API. [V] `cs2cs --s_epoch 2026.9
  EPSG:9989 EPSG:6319` and `projinfo -s "EPSG:9989@2026.9"` work in 9.7.1.
- **PROJ does not move an ITRF point through time by itself.** [V] `projinfo
  -s "ITRF2020@2026.9" -t "ITRF2020@2010.0"` returns "Null geographic offset
  from ITRF2020 to ITRF2020, 0 m". The database has 7 point motion operations
  (Canada velocity grids v6 to v8, Indonesia, New Caledonia, the Nordic uplift
  model) and none for NAD83(2011) or NATRF2022 (`select ... from
  coordinate_operation_view where method_name like 'Point motion%'`).
- **Plate motion is available, but only when asked for by name.** The init
  files `ITRF2008`, `ITRF2014`, `ITRF2020` hold the plate motion models as
  rotation-rate Helmerts (`+init=ITRF2020:NOAM` and 12 other plates; the
  ITRF2020 file since PROJ 9.5.0). [V] file read; applied to the made-up
  point with `cct +proj=helmert +drx=0.000045 +dry=-0.000666 +drz=-0.000098
  +convention=position_vector +t_epoch=2010.0` the North America plate moves
  it 15.7 mm/a west and 5.4 mm/a north in ITRF2020.
- **Deformation beyond rigid plates**: `+proj=deformation` (velocity grid,
  since 5.0), `+proj=defmodel` (multi-component JSON plus GeoTIFF model, since
  7.1), `+proj=tinshift` (7.2). All need a model file. [S] the three pages.
- **Grids**: network access is off by default (`network = off` in
  `/usr/share/proj/proj.ini` [V]); with `PROJ_NETWORK=ON` grids are read in
  chunks from cdn.proj.org into a `cache.db` (300 MB default), since 7.0;
  `projsync` or a mirror of the CDN gives offline use. [S] network page. [V]
  Ubuntu's `proj-data` 9.7.1-1 ships no GeoTIFF grid at all (`dpkg -L
  proj-data`), so GEOID18 (`us_noaa_g2018u0.tif`) and EGM2008 are absent; with
  `PROJ_NETWORK=ON` and a scratch cache directory the GEOID18 lookup worked.
- **A missing grid, a missing operation and the WGS 84 ensemble all fail
  quietly.** [V] (a) Without the geoid grid, `cs2cs EPSG:9989 EPSG:6318+5703`
  returns the NAD83(2011) ellipsoidal height (1.1817 m) as if it were the
  NAVD88 height; with `--only-best` it fails and names the grid; with the grid
  it returns 28.1692 m. (b) `EPSG:4979` ("WGS 84", the ensemble) to
  NAD83(2011) has one candidate, the null transformation EPSG:9774 with 2 m
  accuracy: output equals input. (c) ITRF2020 to NATRF2022 (EPSG:10966 is in
  the database) is only a "ballpark" no-op. [S] ballpark operations and their
  ranking: operations page.
- **WGS 84 realizations are separate CRSs**: G730 to G2296 are in the
  database; [V] EPSG:10608 "WGS 84 (G2296) to ITRF2020" is a no-op with 0.01 m
  accuracy, and `projinfo -s EPSG:10604 -t EPSG:6317` composes it with
  EPSG:10334.

**Relevance**: PROJ already holds everything #27 asked for between ITRF
realizations and NAD83(2011), offline, and GeoZui links it through GDAL. What
it does not do is refuse: the wrapper has to supply the epoch, name the
realization and turn the quiet fallbacks into errors.

---

## PROJ against an independent implementation, and NGS HTDP (question 2)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: [`datums.py` in rolker/ice_cube](https://github.com/rolker/ice_cube/blob/main/ha-addon/gnss_base/gnss_base/datums.py) (commit `f7228c2`), [HTDP user guide 3.6.0](https://geodesy.noaa.gov/TOOLS/Htdp/HTDP-user-guide.pdf), [HTDP revision log](https://geodesy.noaa.gov/TOOLS/Htdp/HTDP-log.pdf), [HTDP page](https://geodesy.noaa.gov/TOOLS/Htdp/Htdp.shtml)

Key takeaways:
- **PROJ and `datums.py` agree to 0.001 mm.** [V] `datums.py` (pure Python;
  ITRF2020 to ITRF2008 with the IERS parameters at epoch 2015.0, then ITRF2008
  to NAD83(2011) with the NGS parameters at epoch 1997.00) was run with
  `python3 -I` and compared with `cs2cs -d 6 EPSG:9988 EPSG:6317` on the
  made-up point. ITRF2020 at 2026.9 to NAD83(2011):

  | | X (m) | Y (m) | Z (m) |
  |---|---|---|---|
  | `datums.py` | 1521040.293981 | -4417420.043405 | 4327501.691666 |
  | PROJ, EPSG:10334 | 1521040.293981 | -4417420.043406 | 4327501.691666 |

  The shift itself is 0.5612 m east, -1.1942 m north, 1.1817 m up (0.2712,
  -1.0725, 1.1927 at epoch 2010.0). To ITRF2014 and ITRF2008 the two also
  agree within 0.001 mm (shifts of 1 to 4 mm). PROJ chained the `datums.py`
  way (9988 to 5332 to 6317) and through ITRF2014 (9988 to 7789 to 6317)
  matches its direct EPSG:10334 result within 0.001 mm. `datums.py`'s inverse
  round trip closes to under a nanometre.
- **Two different parameter sets, one transformation.** `datums.py` chains
  two published sets; PROJ uses EPSG:10334, which EPSG describes as a
  concatenation of the joint US-Canada NAD83(CORS96) to ITRF96 transformation
  with the IGS value for ITRF96 to ITRF97 and the IERS values up to ITRF2020
  ([V] `helmert_transformation.description` in `proj.db`). So the agreement
  checks the arithmetic and the conventions, not two independent
  determinations. [I]
- **HTDP uses the same numbers.** [S] HTDP user guide 3.6.0, Table 5: NAD
  83(2011) with respect to ITRF2020 at epoch 2010.0 is T = 1003.90, -1909.61,
  -541.17 mm; R = 26.78138, -0.42027, 10.93206 mas; s = -0.05109 ppb; rates
  0.79, -0.70, -1.24 mm/a; 0.06667, -0.75744, -0.05133 mas/a; -0.07201 ppb/a.
  [V] These are digit for digit the `+proj=helmert` values PROJ prints for
  EPSG:10334. The guide's equation 5.1 has the rotation signs of PROJ's
  `coordinate_frame` and of `datums.py`'s `sign = -1`.
- **The rotation sign is the trap.** [V] Using the ITRF2008 to NAD83(2011)
  parameters with the IERS sign instead moves the made-up point 0.45 m west
  and 1.36 m south of the right answer.
- **HTDP does more than the Helmert.** [S] user guide sections 1 and 5: it
  first moves the point from one date to another in the source frame with
  velocity grids, 52 plate models and earthquake models, then applies the
  14-parameter Helmert; "the ISOGR and EPSG transformations ... will give
  results that are consistent with HTDP only if the source and target epochs
  are the same". Version 3.6.0 (2025-04-07) treats WGS 84 (G2296), IGS20,
  IGb20 and ITRF2020-u2023 as equal to ITRF2020 (revision log). It says to
  use NCAT, not HTDP, between NAD 83 realizations.
- **What "NAD83(2011) epoch 2010.00" still needs.** The Helmert at the epoch
  of observation gives NAD83(2011) at that epoch; getting to 2010.00 needs the
  point's own NAD83 velocity (HTDP's first step). [V] For a point riding the
  ITRF2020 North America plate model, the made-up point moves 24 mm east and
  31 mm south in NAD83(2011) between 2010.0 and 2026.9 (plate motion from
  `+init=ITRF2020:NOAM` values, plus the change of the EPSG:10334 shift). [I]
  That residual is a property of the model pair, not a measured velocity;
  HTDP's grid would give a different, better number.
- **Not run**: HTDP itself (not installed; its web form is interactive), so
  the HTDP comparison is from its documentation only.

**Relevance**: `datums.py` is confirmed as a correct reference and a ready
source of test values for a PROJ-based implementation; the test should pin
both the numbers above and the sign convention.

---

## How a coordinate's frame and epoch are carried in data (question 3)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: [GDAL coordinate epoch](https://gdal.org/en/stable/user/coordinate_epoch.html), [OGC WKT2:2019 (18-010r11)](https://docs.ogc.org/is/18-010r11/18-010r11.pdf), [RINEX 4.02](https://files.igs.org/pub/data/format/rinex_4.02.pdf), [RTKLIB `solution.c`](https://github.com/tomojitakasu/RTKLIB/blob/master/src/solution.c), [RTKLIB `rtcm3.c`](https://github.com/tomojitakasu/RTKLIB/blob/master/src/rtcm3.c), [BKG NTRIP client changeset 10534](https://software.rtcm-ntrip.org/changeset/10534/ntrip), [RTCM coordinate system messages in one vendor's field software](https://help.fieldsystems.trimble.com/trimble-access/latest/en/gnss-survey-broadcast-rtcm.htm), [geographic_msgs/GeoPoint](https://github.com/ros-geographic-info/geographic_info/blob/master/geographic_msgs/msg/GeoPoint.msg)

Key takeaways:
- **WKT2:2019** [S]: a dynamic CRS is marked `DYNAMIC[FRAMEEPOCH[...]]`,
  optionally with `MODEL["..."]` for a deformation model; the coordinate epoch
  "is not part of a CRS definition", it is metadata of the coordinates,
  mandatory for a dynamic CRS, written `COORDINATEMETADATA[<crs>,
  EPOCH[2016.47]]`; "In high accuracy applications, datum ensembles should not
  be used; individual reference frames should be identified."
- **GDAL** (since 3.4) [S]: `OGRSpatialReference::SetCoordinateEpoch()`,
  `GetCoordinateEpoch()`, `IsDynamic()`; stored in GeoTIFF (GeoKey 5120),
  GeoPackage, FlatGeobuf, JPEG2000, PAM `.aux.xml` and VRT; one epoch per
  dataset or layer ("a dataset containing observations or points with
  different epochs cannot be handled"), though the transformation accepts a
  time per vertex. [V] with the Python bindings: `IsDynamic()` is true for
  EPSG:4326, 4979, 9989, 7912 and false for EPSG:6319; `TransformPoint` from
  EPSG:9989 to EPSG:6319 with time 2026.9 per vertex, or with
  `SetCoordinateEpoch(2026.9)` on the source, gives 42.999989251,
  -70.999993118, 1.1817 (the PROJ and `datums.py` answer); with no epoch it
  gives the epoch 2010.0 answer without complaint.
- **RINEX 4.02** [S]: `APPROX POSITION XYZ` is the "geocentric approximate
  marker position (Units: Meters, Frame: ITRF recommended)"; no realization or
  epoch field. `MARKER TYPE` separates fixed monuments from moving platforms.
- **RTCM 3 station messages**: 1005 and 1006 carry the antenna reference
  point in ECEF at 0.1 mm resolution and a 6-bit "ITRF realization year"
  field, no epoch; RTKLIB stores that field and does not use it ([S] RTKLIB
  `rtcm3.c`, read as a description of the layout; the RTCM standard itself is
  not free and was not read). 1021 to 1027 carry a transformation to a local
  datum and projection (Helmert or Molodensky-Badekas, residual grids); 1300
  carries a "service CRS" name with a coordinate epoch, 1301 a 14-parameter
  Helmert with reference epoch, 1302 an "RTCM CRS" with plate number and
  database links ([S] BKG changeset and the vendor page; secondary). RTKLIB
  marks 1021 to 1027 "not supported" and has no 1300 series.
- **RTKLIB solution files** [S] `solution.c`: the header says
  `lat/lon/height=WGS84/ellipsoidal` (or `geodetic` when a geoid is applied),
  `x/y/z-ecef=WGS84` or `e/n/u-baseline=WGS84`. "WGS84" is a fixed label: no
  realization, no epoch. [I] A relative solution is really in the frame and
  epoch of the base coordinate it was given; a PPP solution in the frame of
  the orbit products.
- **ROS** [V] installed definitions read: `sensor_msgs/NavSatFix` is
  "specified using the WGS 84 reference ellipsoid", altitude above that
  ellipsoid, `header.frame_id` is the antenna's frame on the vehicle;
  `geographic_msgs/GeoPoint` says the same. Neither has a field for a
  realization or an epoch.

**Relevance**: the file formats GeoZui reads through GDAL can carry one epoch
per dataset; GNSS solution files and ROS messages carry none, so frame and
epoch have to come from the loader's configuration or a sidecar, and must be
recorded as "declared by the user", never guessed.

---

## Which realization and epoch position sources use (question 4)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: [IGSMAIL-8444, WGS 84 (G2296)](https://lists.igs.org/pipermail/igsmail/2024/008440.html), [HTDP revision log](https://geodesy.noaa.gov/TOOLS/Htdp/HTDP-log.pdf), [OPUS: about](https://geodesy.noaa.gov/OPUS/about.jsp), [CSRS-PPP updates](https://webapp.csrs-scrs.nrcan-rncan.gc.ca/geod/tools-outils/ppp-info.php), [OPUS Projects guide, appendix F](https://geodesy.noaa.gov/OPUS-Projects/user_guide/_appendixF.html), [NGS new datums FAQ](https://geodesy.noaa.gov/datums/newdatums/FAQNewDatums.shtml)

Key takeaways:
- **GPS broadcast orbits: WGS 84 (G2296)**, "aligned to the ITRF2020", in
  NGA's orbit and clock products from 2024-01-07 [S] IGSMAIL-8444. The HTDP
  log lists it as used "from 1/7/2024 through present. Aligned to ITRF2020 at
  all epochs"; before it G2139 (ITRF2014). EPSG gives the tie 0.01 m at epoch
  2024.0, up to 0.02 m at other epochs ([V] `proj.db`, EPSG:10608). [I] A
  single-receiver fix is therefore in the current ITRF at the epoch of
  observation, but its metre-level accuracy makes the realization moot.
- **IGS products (PPP)**: IGS20 from 2022-11-27, IGb20 from 2025-02-02 [S]
  HTDP log. CSRS-PPP's ITRF output followed: IGS20 for data from 2022-11-27,
  IGb20 from February 2025, IGc20 from January 2026 [S] CSRS-PPP page. [I]
  PPP results are at the epoch of observation; the page read here does not
  state the output epoch or the NAD83(CSRS) option, so that part is unchecked.
- **OPUS** [S]: "All initial computations are performed in ITRF"; "Your final
  ITRF reference frame coordinates retain this observed epoch"; "your NAD83
  coordinates are transformed again to the standard epoch date of January 1,
  2010"; NAVD88 heights use GEOID18. Its sample report names ITRF2014 and
  NAD_83(2011); whether OPUS has moved to ITRF2020 was not confirmed.
- **Network RTK / NTRIP**: a rover's position is in whatever frame and epoch
  the base or network coordinates are in. United States real-time networks
  are tied to NAD83(2011) epoch 2010.00, and the OPUS Projects guide warns
  that a network's correctors may still disagree with the published
  coordinates at passive marks ([S] appendix F; found by search summary, read
  only in part). A self-surveyed or PPP-derived base is in ITRF at its
  observation epoch. [I] The NTRIP sourcetable does not state the frame; the
  only in-band statements are the RTCM fields in the previous entry.
- **Other constellations**: GLONASS PZ-90.11 is reported aligned to the ITRF
  at the centimetre level; Galileo (GTRF) and BeiDou were not checked.
  Secondary sources only.
- **The frame is about to change in the United States.** NATRF2022 and
  NAPGD2022 are in beta, reference epoch 2020.00, defined relative to
  ITRF2020, and not yet official (search results of 2026-11; NGS FAQ; dates
  quoted by third parties vary). [V] PROJ 9.7.1 knows the name (EPSG:10966)
  but has no transformation to it.

**Relevance**: the ice-growth case in #27 mixes exactly these: a base in
ITRF2020 at the epoch of observation, a regional network in NAD83(2011) at
2010.00 (1.3 m apart horizontally at the made-up point), and PPP or OPUS
results in either.

---

## Vertical: ellipsoid height, geoid models and tidal datums (question 5)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: [VDatum: datums](https://vdatum.noaa.gov/docs/datums.html), [OPUS: about](https://geodesy.noaa.gov/OPUS/about.jsp), [PROJ network and grids](https://proj.org/en/stable/usage/network.html), `marine_vertical_datum` and ADR-0010 in [rolker/unh_marine_autonomy](https://github.com/rolker/unh_marine_autonomy) at `0ca4a587` (read locally: `README.md`, `include/marine_vertical_datum/vdatum_query.hpp`, `src/vdatum_query.cpp`, `docs/decisions/0010-geospatial-world-model.md` D5 and D6)

Key takeaways:
- **An ellipsoidal height belongs to a frame as much as a latitude does.**
  [V] At the made-up point the ITRF2020 height 0 is 1.1817 m in NAD83(2011)
  at epoch 2026.9 (1.1927 m at 2010.0): the frames' origins differ by about
  2 m. The GRS80 and WGS 84 ellipsoids themselves differ negligibly ([S]
  VDatum page).
- **Each geoid model is tied to one frame.** [V] In `proj.db`, GEOID18
  (`us_noaa_g2018u0.tif`, EPSG:9229, 0.015 m) goes from NAD83(2011)
  (EPSG:6319) to NAVD88 height; EGM2008 (`us_nga_egm08_25.tif`) from WGS 84
  (EPSG:4979). [V] With the grid fetched over the network, the made-up point
  taken as ITRF2020 at 2026.9 gets NAVD88 height 28.1692 m (PROJ inserts
  EPSG:10334 before the grid); the same numbers taken as NAD83(2011) get
  26.9875 m. The difference is the 1.1817 m above.
- **Tidal datums hang off the orthometric datum**: VDatum treats ellipsoidal,
  orthometric and tidal datums as three connected classes; tidal datums are
  local, averaged over the 19-year National Tidal Datum Epoch (1983 to 2001)
  [S] VDatum page. The page read does not describe the NAVD88 to local mean
  sea level step; that is from the library below.
- **What `marine_vertical_datum` covers** (read, not run): a ROS-free library
  answering "what is MLLW (optionally MHHW) relative to the WGS84 ellipsoid at
  this point"; PROJ context with networking disabled; one geoid grid plus
  NOAA VDatum regional `.gtx` grids found by file name; a precedence chain
  (lake parameter, override polygons, VDatum, fallback polygons, absent);
  headers free of PROJ. ADR-0010 D5: stored heights are WGS84-ellipsoidal and
  conversion happens once, at import; D6: the library and where grids live.
- **What it does not cover.** Its pipeline is two steps, `+proj=vgridshift`
  with the geoid and `+proj=vgridshift` with the datum grids
  (`create_pipeline` in `src/vdatum_query.cpp`), with no Helmert step and no
  epoch. [I] So with GEOID18 it treats its "WGS84 ellipsoid" input as
  NAD83(2011); if the stored heights are really from GNSS in a current ITRF,
  the chart datum comes out wrong by the frame difference in height (1.18 m
  at the made-up point; the size elsewhere, and which frame the stored
  heights are actually in, were not checked). This is the vertical face of
  the gap #27 records and should be confirmed with its owner before anyone
  relies on this reading.
- NAPGD2022 with GEOID2022 is planned to replace NAVD88 (see previous entry).

**Relevance**: "store ellipsoidal, convert at import" (#27) is only complete
when it says which ellipsoidal frame. A shared datum library has to do the
horizontal and vertical steps in one pipeline, as PROJ does when given named
CRSs.

---

## Recommendations and open questions for the design discussion (question 6)

**Added**: 2026-11-24 | **Last verified**: 2026-11-24 | **Sources**: the five entries above; issues [#27](https://github.com/rolker/geozui4d/issues/27) and [#28](https://github.com/rolker/geozui4d/issues/28)

Everything in this entry is [I]: recommendations drawn from the findings
above, for the owner to decide.

Recommendations:
- **Make PROJ the authority, behind a small ROS-free library** in the manner
  of `marine_vertical_datum`: PROJ a private dependency, PROJ-free headers,
  networking off, built once and queried per point. PROJ's results match
  `datums.py` to 0.001 mm and carry NGS's own parameters.
- **The unit of data is (frame, epoch, coordinate)**, where the frame is a
  named realization (ITRF2020, ITRF2014, ITRF2008, NAD83(2011), a WGS 84
  G-realization) and never bare "WGS84". This matches WKT2 coordinate
  metadata and GDAL's coordinate epoch.
- **Refuse rather than guess.** The library should fail when: a dynamic frame
  comes without an epoch (PROJ would use the operation's reference epoch);
  the source or target is the WGS 84 ensemble or otherwise unnamed (PROJ
  would return the input unchanged); the only operation is a ballpark one or
  a grid is missing (use `ONLY_BEST` and disallow ballpark operations); two
  datasets in different frames or epochs are combined without a stated
  transformation; a change of epoch is asked for without a named motion
  model.
- **Keep "change frame at the same epoch" and "move in time" as two separate
  calls.** The first is fully served by EPSG operations in PROJ. The second
  needs a named model (a plate from the ITRF plate motion model, a velocity
  grid, or HTDP run outside) and is where the judgement lies.
- **Report what was done**: the operation's name and EPSG code, its stated
  accuracy, the epoch used and any grid, so a scene can show it
  (`proj_trans_get_last_used_operation`, PROJ 9.1 and later).
- **Tests from `datums.py`**: pin the made-up-point numbers in this digest at
  two epochs, both directions, and one case with the rotation sign reversed
  that must not match.
- **Vertical in the same pipeline**: let PROJ compose frame change and geoid
  from named CRSs instead of applying a geoid grid to an unnamed height.
- **Grids are provisioned, not fetched**: follow ADR-0010 D6 (offline, a
  known directory), and fail loudly when one is absent.

Open questions:
- Which PROJ is the minimum? Ubuntu 26.04 has 9.7.1 here; Ubuntu 24.04's
  version was not checked, and `proj_crs_is_dynamic()` needs 9.9.
- Call PROJ directly or through GDAL's `OGRCoordinateTransformation`? GeoZui
  already links GDAL; the other project's library links PROJ.
- Where does the library live so that both projects can depend on it, and
  does `marine_vertical_datum` grow into it or sit beside it?
- What is the canonical stored frame: the frame of observation kept as
  metadata, or everything brought to one frame and epoch at import (which one,
  given NATRF2022 at epoch 2020.00 is coming)?
- Is a rigid-plate model enough for moving between epochs at the sites of
  interest, or is HTDP's velocity grid (or NGS's coming models) needed? The
  two were not compared here; the rigid-plate model alone leaves about 4 cm
  of motion in NAD83(2011) over 17 years at the made-up point.
- Are the heights `marine_vertical_datum` receives in NAD83(2011) or in a
  current ITRF? (See the vertical entry.)
- How do loaders learn the frame and epoch of a solution file or a ROS
  message that cannot carry them: a sidecar, a scene attribute, or a refusal
  to load without one?
- Not answered by this research: the text of the RTCM standard for messages
  1005/1006 and 1300 to 1304; the output epoch of the PPP services; Galileo
  and BeiDou frames; a run of HTDP itself.

**Relevance**: input to the design discussion that #27 opens; nothing here
is decided.

---
