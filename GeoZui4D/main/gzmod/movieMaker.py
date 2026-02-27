#!/usr/bin/env python
# gz
# movieMaker
# Setup and record movies from GeoZui scenes.

import gz4d.MovieMaker

if __name__ == '__main__':
	app = gz4d.MovieMaker.MovieMaker()
	app.MainLoop()
