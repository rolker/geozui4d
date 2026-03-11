#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import sys
import os
import time

sys.path.append(os.path.join(sys.path[0],'../lib/python'))

import gz4d

def usage():
    print 'usage: simradME70.py [key=val ...]'
    print '    options may be:  id         Id of server    (0)'
    print '                     port       Remote port     (37655)'
    print '                     laddress   Local address   (ANY)'
    print '                     raddress   Remote address  (BROADCAST)'

options = {'id':0, 'port':37655, 'laddress':'ANY', 'raddress':'BROADCAST'}

for a in sys.argv[1:]:
    opt = a.split('=',1)
    if len(opt) == 2 and opt[0] in options.keys():
        options[opt[0]]=opt[1]
    else:
        usage()
        sys.exit(1)

class ShowNav:
    def __init__(self, p):
        self.lat = p['position']['lat']
        self.lon = p['position']['lon']
        self.queue = gz4d.utilities.TimeThreadedQueue()
        p['position'].AddOutQueue(self.queue)

    def __str__(self):
        return str(self.lat.Get(0.0))+', '+str(self.lon.Get(0.0))+'\t'+str(self.lat.GetLatestTime())+', '+str(self.lon.GetLatestTime())

    def CheckQueue(self):
        while not self.queue.empty():
            t = self.queue.pop()
            print t,self.lat.Get(0.0,t),self.lon.Get(0.0,t)

class TargetOut:
    def __init__(self, label, subscription):
        self.label = label
        self.queue = gz4d.sonar.simrad.TargetSetThreadedQueue()
        subscription.AddOutQueue(self.queue)

    def CheckQueue(self):
        while not self.queue.empty():
            ts = self.queue.pop()
            print ts.time,self.label
            for t in ts.targets:
                print t.position, t.value

class SamplesOut:
    def __init__(self, label, subscription):
        self.label = label
        self.queue = gz4d.sonar.simrad.SampleSetThreadedQueue()
        subscription.AddOutQueue(self.queue)

    def CheckQueue(self):
        while not self.queue.empty():
            ss = self.queue.pop()
            print ss.time,self.label,len(ss.samples),'samples'



sm = gz4d.sonar.simrad.ServerManager()
time.sleep(0.5)
sl = sm.GetList()
for s in sl:
    print s

s = sm.Get(int(options['id']))
if s:
    client = s.CreateClient()
    client.Connect()

    p = client.GetPlatform()
    params = p.GetParameters()

    n = ShowNav(params)

    channels = client.GetTransducer().GetChannels()

    bottoms = []
    samples = []

    for c in channels:
        print c
        if len(bottoms) < 2:
            bottoms.append(TargetOut('Bottom: '+str(c),c.GetBottomDetection()))
            samples.append(SamplesOut('Samples: '+str(c),c.GetSamples()))

    for i in range(150):
        n.CheckQueue()
        for b in bottoms:
            b.CheckQueue()
        for s in samples:
            s.CheckQueue()
        time.sleep(.1)

else:
    print 'server not found'

