#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import gz4d
import gz4d.qt

import sys

if(len(sys.argv) < 2):
    print 'usage: qt_x3d model.x3d'
    sys.exit(1)

x3d = gz4d.x3d.X3D()
x3d.Load(sys.argv[1])

app = gz4d.qt.Application()
app.Create3DViewer(x3d)
app()
