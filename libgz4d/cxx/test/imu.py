#!/usr/bin/env python

import pylab
import csv
import sys

class Kalman:
    def __init__(self):
	self.estimate = None
	self.error = None

    def Add(self, value):
	# predict
	pass

class AlphaBetaFilter:
    def __init__(self, x=None, dx=None, alpha = .1, beta=.1):
	self.x = x
	self.dx = dx
	self.alpha = alpha
	self.beta = beta

    def Add(self, value):
    	if self.dx is None and self.x is not None:
	    self.dx = value-self.x
	    self.x = value
	    return value

	if self.x is None:
	    self.x = value
	    return value

	ex = self.x + self.dx

	r = value - ex

	self.x  = ex  + self.alpha*r
	self.dx += self.beta*r

	return self.x

data = csv.reader(file(sys.argv[1]))

try:
    maxc = int(sys.argv[2])
except IndexError:
    maxc = None

times = []
ftimes = []

f = AlphaBetaFilter(dx=0.01,alpha=.001,beta=.000001)
dxs = []

period = 0.01

lastc = None
for r in data:
    ts = float(r[0])
    c = int(r[1])

    if lastc is not None:
        if c != lastc+1 and c != 0:
            print 'Skipped!',lastc,'to',c
    lastc = c
    
    times.append(ts)
    ftimes.append(f.Add(ts))

    p = f.dx
    if p is not None:
        while len(dxs) < len(times):
            dxs.append(p)

    if maxc is not None and len(times) >= maxc:
        break

a = (ftimes[-1]-ftimes[0])/(len(ftimes)-1)

adp = 0.0
for dp in dxs:
    adp = dp/len(dxs)

#a = period+adp

avgt = 0.0
for t in ftimes:
    avgt += t/len(ftimes)

b = avgt - a*(len(ftimes)-1)/2.0

eq = 'an+b (a='+str(a)+', b='+str(b)+')'

for i in range(len(times)):
    t = a*i+b
    times[i] -= t
    ftimes[i] -= t

#pylab.subplot(311)
pylab.plot(times,label="raw time re an+b")

#pylab.subplot(312)
pylab.plot(ftimes,label="filtered time re an+b")

#pylab.subplot(313)
pylab.plot(dxs,label="delta time used in filter")
pylab.legend()
pylab.ylabel('s')
pylab.xlabel('n')
pylab.title(eq)
pylab.show()