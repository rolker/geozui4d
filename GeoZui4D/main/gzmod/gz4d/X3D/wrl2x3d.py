#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import vrml
import sys

if len(sys.argv) != 2:
    print 'usage: wrl2x3d.py in.wrl'
    sys.exit(1)

f = open(sys.argv[1])
scene = vrml.Scene()
scene.parse(f)

print scene.x3dString()
