#!/usr/bin/env python

import sys
import os
import time
import socket

sys.path.append(os.path.join(sys.path[0],'../lib/python'))

import gz4d

navTrack = gz4d.nav.Track()
navSync = gz4d.nav.Synchronizer(navTrack,.5)

navSyncThread = gz4d.nav.SynchronizerConsumer(navSync)

navQueue = gz4d.nav.FixThreadedQueue()
rawNavQueue = gz4d.nav.FixThreadedQueue()

navTrack.AddOutQueue(navSyncThread.GetInQueue())
navTrack.AddOutQueue(rawNavQueue)

navSync.AddOutQueue(navQueue)

navThread = gz4d.nav.TrackConsumer(navTrack)

for fn in sys.argv[1:]:
    s = gz4d.sonar.simrad.FileSource(fn)
    print s

    s.AddNavOutQueue(navThread.GetInQueue())
    

    while s.ProcessNextDatagram():
        while not rawNavQueue.empty():
            print 'Async:',rawNavQueue.pop()

        while not navQueue.empty():
            print 'Sync:',navQueue.pop()
