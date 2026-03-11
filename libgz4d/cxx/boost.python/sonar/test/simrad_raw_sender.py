#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import sys
import os
import time
import socket

sys.path.append(os.path.join(sys.path[0],'../lib/python'))

import gz4d
import GZConnection

gz = GZConnection.GZConnection()
gz.Connect()

ret = gz.Send('gz newobject BasicVessel vessel')
oid = ret[1].strip()
print 'oid:',oid
gz.Send('gz tellObject '+oid+' projection Geographic')
gz.Send('gz tellObject '+oid+' setRT')
ret = gz.Send('gz tellObject ' + oid + ' connect')
port = int(ret[1])
print 'port:',port

navSender = gz4d.nav.FixUDPSender(gz4d.socket.IPAddress('localhost',port))

navTrack = gz4d.nav.Track()
navSync = gz4d.nav.Synchronizer(navTrack,.5)

navSenderThread = gz4d.nav.FixUDPSenderConsumer(navSender)
navSync.AddOutQueue(navSenderThread.GetInQueue())

navSyncThread = gz4d.nav.SynchronizerConsumer(navSync)

navTrack.AddOutQueue(navSyncThread.GetInQueue())
navThread = gz4d.nav.TrackConsumer(navTrack)

#ret = gz.Send('gz newobject XYZStream bottom')
#oid = ret[1].strip()
#print 'oid:',oid
#cellsize = '5.0'
#gz.Send('gz tellObject ' + oid + ' initialize ' + cellsize + ' swa ' + cellsize)
#ret = gz.Send('gz tellObject ' + oid + ' connect')
#port = int(ret[1])
#print 'port:',port
#bottomSocket = gz4d.socket.UDPSocket()
#bottomSocket.Connect(gz4d.socket.IPAddress('localhost',port))

for fn in sys.argv[1:]:
    s = gz4d.sonar.simrad.FileSource(fn)
    s.AddNavOutQueue(navThread.GetInQueue())

    while s.ProcessNextDatagram():
        pass

navThread.WaitEmpty()
navSyncThread.WaitEmpty()
navSenderThread.WaitEmpty()
