#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# movieMaker
# Setup and record movies from GeoZui scenes.

import gz4d.MovieMaker

if __name__ == '__main__':
	app = gz4d.MovieMaker.MovieMaker()
	app.MainLoop()
