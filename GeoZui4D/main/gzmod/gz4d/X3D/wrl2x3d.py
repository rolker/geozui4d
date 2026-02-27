#!/usr/bin/env python

import vrml
import sys

if len(sys.argv) != 2:
    print 'usage: wrl2x3d.py in.wrl'
    sys.exit(1)

f = open(sys.argv[1])
scene = vrml.Scene()
scene.parse(f)

print scene.x3dString()
