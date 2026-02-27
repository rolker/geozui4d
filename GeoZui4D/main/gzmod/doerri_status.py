#!/usr/bin/env python
# gz
# doerri
# Get Doerri status

import sys
import gz4d
import datetime
import calendar
import socket

zone = None

gz = gz4d.Connection()
gz.Connect()
		
s = socket.socket(type=socket.SOCK_DGRAM)
s.bind(('',2056))

now = datetime.datetime.utcnow()
fname = '-'.join((str(now.year),str(now.month),str(now.day),str(now.hour),str(now.minute),str(now.second),'doerri.log'))
log = open(fname,'w')

lastGPSTime = None

while True:
    status = s.recv(1024)
    now=datetime.datetime.utcnow()
    t = calendar.timegm(now.utctimetuple())+now.microsecond/1000000.0
    log.write(str(t)+','+status)
    parts = status.split()
    print t,parts
    if len(parts) >=40 and parts[0][-7:] == 'Heading':
        gpsTime = parts[39]
        if gpsTime != '0' and gpsTime != lastGPSTime:
            lastGPSTime = gpsTime
            try:
                hour = int(gpsTime[-6:-4])
            except ValueError:
                hour = 0
            try:
                minute = int(gpsTime[-4:-2])
            except ValueError:
                minute = 0
            try:
                second = int(gpsTime[-2:])
            except ValueError:
                second = 0
            gt = datetime.datetime(now.year,now.month,now.day,hour,minute,second)
            t = calendar.timegm(gt.utctimetuple())
            h=parts[1]
            lat = parts[3]
            lon = parts[4]
            if zone is None:
                result = gz.SendCommand('gz projection init; gz projection longToUTM '+lon)
                zone = result[1]
                print zone
            result = gz.SendCommand(' '.join(('gz projection apply',zone,lon,lat)))
            #print result[1]
            en = result[1].split()
            x = float(en[0])
            y = float(en[1])
            #print 'doerri',x,y
            p = parts[23]
            r = parts[25]

            gz.SendCommand(' '.join(('gz tellObject Doerri setPositionOrientationTime',str(x),str(y),'0.0',h,p,r,str(t))))
    # gz tellObject gps setPositionOrientationTime x y z h p r t