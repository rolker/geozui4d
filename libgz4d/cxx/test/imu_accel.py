#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import pylab
import csv
import sys
import numpy

for f in sys.argv[1:]:
    pylab.figure()
    pylab.title(f)

    for o,t in ((0,'mag'),(3,'accel'),(6,'gyro')):
        data = csv.reader(file(f))

        xs = []
        ys = []
        zs = []
        
        for r in data:
            ax = int(r[2+o])
            ay = int(r[3+o])
            az = int(r[4+o])
        
            xs.append(ax)
            ys.append(ay)
            zs.append(az)
        
        x = (numpy.average(xs),numpy.var(xs))
        y = (numpy.average(ys),numpy.var(ys))
        z = (numpy.average(zs),numpy.var(zs))
        
        
        print f,t,x,y,z
        
        pylab.plot(xs,label=t+' x')
        pylab.plot(ys,label=t+' y')
        pylab.plot(zs,label=t+' z')
    pylab.legend()

pylab.show()
