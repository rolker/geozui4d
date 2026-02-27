#!/usr/bin/env python

import pylab
import csv
import sys
import numpy
import math
import scipy.optimize
import kalman

data = csv.reader(file(sys.argv[1]))

xs = []
ys = []
zs = []

kxs = []
kys = []
kzs = []

for r in data:
    magx = int(r[2])
    magy = int(r[3])
    magz = int(r[4])

    if len(xs) == 0:
        xk = kalman.SimpleKalman(magx,0.0,0.0,0.0005)
        yk = kalman.SimpleKalman(magy,0.0,0.0,0.0005)
        zk = kalman.SimpleKalman(magz,0.0,0.0,0.0005)

    xs.append(magx)
    ys.append(magy)
    zs.append(magz)

    kxs.append(xk.Add(magx,numpy.var(xs[-15:])))
    kys.append(yk.Add(magy,numpy.var(ys[-15:])))
    kzs.append(zk.Add(magz,numpy.var(zs[-15:])))


pylab.plot(xs,'r',label='x',alpha=0.25)
pylab.plot(ys,'g',label='y',alpha=0.25)
pylab.plot(zs,'b',label='z',alpha=0.25)

pylab.plot(kxs,'r',label='x (kalman)')
pylab.plot(kys,'g',label='y (kalman)')
pylab.plot(kzs,'b',label='z (kalman)')

pylab.legend()




p0 = [numpy.average(kxs),numpy.average(kys),numpy.average(kzs),(max(kxs)-min(kxs))/2,(max(kys)-min(kys))/2,(max(kzs)-min(kzs))/2]


def ei(p,xs,ys,zs):
    ret = []
    for x,y,z in zip(xs,ys,zs):
        xi = (x-p[0])**2
        yi = (y-p[1])**2
        zi = (z-p[2])**2
        a2 = p[3]**2
        b2 = p[4]**2
        c2 = p[5]**2
        ret.append(math.sqrt(xi+yi+zi)* (1-1/(xi/a2+yi/b2+zi/c2)))
    return ret

p = scipy.optimize.leastsq(ei,p0,args=(kxs,kys,kzs))

x0 = p[0][0]
y0 = p[0][1]
z0 = p[0][2]
a = p[0][3]
b = p[0][4]
c = p[0][5]

print x0,y0,z0,a,b,c

#e = ei(p[0],xs,ys,zs)
#print 'avg err',numpy.average(e)

cxs = []
cys = []
czs = []

for x,y,z in zip(kxs,kys,kzs):
    cxs.append((x-x0)/a)
    cys.append((y-y0)/a)
    czs.append((z-z0)/a)

pylab.figure()

pylab.plot(cxs,'r',label='x')
pylab.plot(cys,'g',label='y')
pylab.plot(czs,'b',label='z')

pylab.show()
