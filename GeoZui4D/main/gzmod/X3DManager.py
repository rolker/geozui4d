#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# X3DManager
# Load X3D and VRML file into GeoZui4D

import gz4d.X3D.x3dManagerApp

if __name__ == '__main__':
	app = gz4d.X3D.x3dManagerApp.x3dManagerApp()
	app.MainLoop()
