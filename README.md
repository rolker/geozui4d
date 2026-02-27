# GeoZui4D

GeoZui4D is a 4-dimensional geographic visualization system developed at the
Data Visualization Research Lab (VisLab), Center for Coastal and Ocean Mapping
(CCOM), University of New Hampshire. The "4D" refers to 3D spatial
visualization plus time as the fourth dimension.

It is a real-time, interactive geospatial visualization platform designed
primarily for ocean mapping, marine navigation, and underwater exploration.

## History

GeoZui3D -- "Geographic Zooming User Interface" -- has its roots in the
Fledermaus visualization system developed at the University of New Brunswick
by Colin Ware and others. Fledermaus was spun off as a commercial product
through Interactive Visualization Systems (IVS), now part of QPS through
subsequent acquisitions. When Colin moved to the University of New Hampshire
and established the Data Visualization Research Lab (VisLab) at CCOM, GeoZui3D
was born around 2000 as a research platform continuing and expanding on that
earlier work. Over the years, Fledermaus in turn adopted a number of ideas
from GeoZui.

The "4D" came with the addition of time as a fourth dimension, enabling
temporal data playback, time-varying surfaces, and animated vessel tracks.

### Selected Publications

- C. Ware, M. Plumlee, R. Arsenault, L.A. Mayer, S. Smith. "GeoZui3D: Data
  Fusion for Interpreting Oceanographic Data." MTS/IEEE Oceans 2001, Honolulu.
  [doi:10.1109/OCEANS.2001.968146](https://doi.org/10.1109/OCEANS.2001.968146)

- M. Plumlee, C. Ware. "Integrating Multiple 3D Views Through
  Frame-of-Reference Interaction." Coordinated and Multiple Views in
  Exploratory Visualization (CMV 2003), London.
  [doi:10.1109/CMV.2003.1215001](https://doi.org/10.1109/CMV.2003.1215001)

- R. Arsenault, C. Ware, M. Plumlee, S. Martin, L.L. Whitcomb, D. Wiley,
  T. Gross, A. Bilgili. "A System for Visualizing Time Varying Oceanographic
  3D Data." Oceans '04 MTS/IEEE Techno-Ocean '04, Kobe.
  [doi:10.1109/OCEANS.2004.1405535](https://doi.org/10.1109/OCEANS.2004.1405535)

- S.C. Martin, L.L. Whitcomb, R. Arsenault, M. Plumlee, C. Ware. "A System
  for Real-Time Spatio-Temporal 3-D Data Visualization in Underwater Robotic
  Exploration." IEEE ICRA 2005, Barcelona.
  [doi:10.1109/ROBOT.2005.1570345](https://doi.org/10.1109/ROBOT.2005.1570345)

- C. Ware, R. Arsenault, M. Plumlee, D. Wiley. "Visualizing the Underwater
  Behavior of Humpback Whales." IEEE Computer Graphics and Applications,
  26(4), 2006.
  [doi:10.1109/MCG.2006.93](https://doi.org/10.1109/MCG.2006.93)

### Contributors

GeoZui3D/4D was developed primarily by Colin Ware, Roland Arsenault, and
Matt Plumlee, with contributions from Don House (Texas A&M), Briana Sullivan,
Rick Komerska, and other members of the VisLab. Some widget code traces back
to earlier work by Glenn Franck, Nhu Le, and Colin Ware at UNB.

## Capabilities

- **Center of workspace interaction** -- clicking any point moves it to the
  center of the workspace; all zooming and rotation happen around that center.
  This unifies viewpoint control, multi-view linking, and stereoscopic display
- **Bathymetric surface visualization** with height exaggeration and color
  lookup table editing
- **Real-time data streaming** from GPS, NMEA, AIS, and sonar instruments
- **Vessel tracking** with predictors and historical tracks
- **Multibeam sonar visualization** (Reson, Simrad ME70, EK500/EK60, DeltaT)
- **S-57 electronic nautical chart** rendering
- **Ocean current and flow visualization**
- **Time-varying surfaces** for temporal data playback
- **Mission planning** for AUV/survey operations
- **X3D/VRML model loading**
- **Tcl/Tk scripting interface** for automation
- **Multiple GUI modes**: GeoZui4D (full), GeoNav3D (navigation), GeoCoastPilot
  (chart-based), GeoWall2 (immersive stereo display)

### Supported File Formats

Scenes (`.gzx`), bathymetry (`.gutm`, `.dem`, `.grd`, `.bag`, `.srtm`),
vector data (`.gdl`, `.gdp`, `.gdt`, `.dxf`), 3D models (`.x3d`, `.wrl`),
point clouds (`.xyz`, `.pts`), sonar (`.raw`, `.sws`), charts (`.000`/S-57),
images (`.tif`, `.jpg` with world files), NetCDF (`.nc`), audio (`.ogg`,
`.flac`), and more.

## Building

### Prerequisites

On Ubuntu/Debian, install the required packages:

```sh
sudo apt install \
  build-essential cmake \
  libboost-all-dev \
  tcl-dev tk-dev \
  libgdal-dev \
  libjpeg-dev libtiff-dev libpng-dev \
  libnetcdf-dev libnetcdf-cxx-legacy-dev \
  freeglut3-dev libglew-dev \
  libx11-dev \
  portaudio19-dev libjack-dev \
  libavformat-dev libavcodec-dev libavutil-dev \
  libvorbis-dev \
  libbluetooth-dev \
  libspnav-dev
```

### Build

```sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Install and Run

```sh
cd build
cmake --install . --prefix ../install
../install/bin/GeoZui4D [files...]
```

### Alternative: Building Sub-Projects Separately

The sub-projects can still be built independently if needed.

#### Build libgz4d

```sh
mkdir -p /tmp/gz4d-libgz4d-build && cd /tmp/gz4d-libgz4d-build
cmake /path/to/gz4d/libgz4d/cxx \
  -Dais_INCLUDE_DIR=NOTFOUND -Dais_LIBRARY=NOTFOUND
make -j$(nproc)
```

The AIS module is disabled above because the current libais API has diverged.

#### Build GeoZui4D

```sh
mkdir -p /tmp/gz4d-app-build && cd /tmp/gz4d-app-build
cmake /path/to/gz4d/GeoZui4D/main \
  -Dgz4d_INCLUDE_DIR=/path/to/gz4d/libgz4d/cxx/include \
  -Dgz4d_x3d_LIBRARY=/tmp/gz4d-libgz4d-build/src/x3d/libgz4d_x3d.so
make -j$(nproc)
```

#### Install and Run

```sh
cd /tmp/gz4d-app-build
cmake --install . --prefix /tmp/gz4d-install

LD_LIBRARY_PATH=/tmp/gz4d-libgz4d-build/src/x3d \
  /tmp/gz4d-install/bin/GeoZui4D [files...]
```

The install step is required -- GeoZui4D locates its Tcl scripts, bitmaps, and
other support files relative to the executable at `../lib/GeoZui4D/`.

You can pass data files on the command line (`.gzx`, `.x3d`, `.gutm`, etc.)
or load them through the File menu.

Command-line flags:
- `-nav` -- Start in GeoNav3D mode
- `-cp` -- Start in GeoCoastPilot mode
- `-wall` -- Start in GeoWall2 stereo display mode

## Project Structure

```
gz4d/
  libgz4d/
    cxx/
      include/gz4d/   -- Library headers (geo, sonar, netcdf, av, etc.)
      src/             -- Library source (X3D parser)
      test/            -- Test programs
      boost.python/    -- Python bindings via Boost.Python
      CMakeLists.txt
    python/            -- Python utilities (iso8211, S-57, etc.)
  GeoZui4D/
    main/
      engine/          -- Rendering engine, scene management, interactors
      objects/         -- Object types (gutm, vessel, sonar, charts, etc.)
      interactors/     -- Input handling (mouse, spaceball, wiimote)
      Togl/            -- OpenGL widget for Tcl/Tk
      scripts/         -- Tcl extension scripts
      gzmod/           -- Python extension modules
      *.tcl            -- Main Tcl/Tk GUI scripts
      CMakeLists.txt
```

## License

Copyright (c) 2000 Data Visualization Research Lab, Center for Coastal and
Ocean Mapping, University of New Hampshire. All rights reserved.

For research purposes only. See `GeoZui4D/main/license.txt` for details.
