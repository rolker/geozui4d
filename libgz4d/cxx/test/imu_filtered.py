#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import pylab
import csv
import sys
import numpy
import math
import scipy.optimize
import kalman



for o,t in ((0,'mag'),(3,'accel'),(6,'gyro')):
    xs = []
    ys = []
    zs = []
    
    fxs = []
    fys = []
    fzs = []

    data = csv.reader(file(sys.argv[1]))
    for r in data:
        x = float(r[2+o])
        y = float(r[3+o])
        z = float(r[4+o])
    
        if r[1] == 'filtered':
            fxs.append(x)
            fys.append(y)
            fzs.append(z)
        else:
            xs.append(x)
            ys.append(y)
            zs.append(z)

    
    pylab.figure()
    pylab.title(t)
    pylab.plot(xs,'r',label='x',alpha=0.25)
    pylab.plot(ys,'g',label='y',alpha=0.25)
    pylab.plot(zs,'b',label='z',alpha=0.25)
    
    pylab.plot(fxs,'r',label='x (filtered)')
    pylab.plot(fys,'g',label='y (filtered)')
    pylab.plot(fzs,'b',label='z (filtered)')

    pylab.legend()

pylab.show()
