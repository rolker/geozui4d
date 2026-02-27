#!/usr/bin/env python

import pylab
import csv
import sys
import numpy
import kalman

data = csv.reader(file(sys.argv[1]))

times = []
stimes = []
periods = []

f = None

for r in data:
    ts = float(r[0])
    c = int(r[1])

    times.append(ts)

    if f is None:
        f = kalman.SimpleKalman(ts,0.01,0.00719)
        stimes.append(ts)
    else:
        stimes.append(f.Add(ts))
    if c == 15:
        break

avgp = (times[-1]-times[0])/(len(times)-1)
print 'average period',avgp

diffs = []
sdiffs = []

for i in range(len(times)-1):
    diffs.append(times[i+1]-times[i])
    sdiffs.append(stimes[i+1]-stimes[i])
    #diffs.append((times[0]+i*avgp)-times[i])
    #sdiffs.append((times[0]+i*avgp)-stimes[i])

print numpy.average(diffs),numpy.std(diffs)
print f.x
print f.P

pylab.plot(diffs)
pylab.plot(sdiffs)


#pylab.plot(stimes)

pylab.show()
