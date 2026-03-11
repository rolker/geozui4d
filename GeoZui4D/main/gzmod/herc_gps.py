#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# herc_gps
# reads gps data from Hercules and friends

import sys
import gz4d
import datetime
import calendar
import socket


def getSecs(tstr):
    now = datetime.datetime.utcnow()
    try:
        hour = int(tstr[0:2])
        minute = int(tstr[2:4])
        sec = float(tstr[4:])
        isec = int(sec)
        fsec = sec-isec
        t = datetime.datetime(now.year,now.month,now.day,hour,minute,isec)
    except ValueError:
        return None
    return calendar.timegm(t.utctimetuple())+fsec

print 'Starting herc_gps.py'

zone = None
h = {'$PV':0.0,'$M1':0.0,'$M2':0.0}
targets = {'$PV':'Alliance','$M1':'Herc','$M2':'Argus'}

gz = gz4d.Connection()
gz.Connect()

#vislab = socket.socket(type=socket.SOCK_DGRAM)
#vislab.bind(('',0))

gps = socket.socket(type=socket.SOCK_DGRAM)
gps.bind(('',4201))

now = datetime.datetime.utcnow()
fname = '-'.join((str(now.year),str(now.month),str(now.day),str(now.hour),str(now.minute),str(now.second),'ife.log'))
log = open(fname,'w')


while True:
    line = gps.recv(1024)
    log.write(line)
    for l in line.split('$'):
        if len(l) > 0:
            l = '$'+l
            #vislab.sendto(l,('132.177.140.178',6666))
            #print l
            nmea = l.strip().split(',')
            src = nmea[0][:3]
            type = nmea[0][-3:]
            #print type
            if type == 'GGA':
                #print nmea
                try:
                    lat = int(nmea[2][0:2])+float(nmea[2][2:-1])/60.0
                    if nmea[3] == 'S':
                        lat = -lat
                except ValueError:
                    lat = None
                try:
                    lon = int(nmea[4][0:3])+float(nmea[4][3:-1])/60.0
                    if nmea[5] == 'W':
                        lon = -lon
                except ValueError:
                    lon = None
                #print lat,lon
                if lat and lon:
                    if zone is None:
                        result = gz.SendCommand('gz projection init; gz projection longToUTM '+str(lon))
                        zone = result[1]
                        print zone
                    result = gz.SendCommand(' '.join(('gz projection apply',zone,str(lon),str(lat))))
                    #print result[1]
                    en = result[1].split()
                    x = float(en[0])
                    y = float(en[1])
                    try:
                        z = float(nmea[9])
                    except ValueError:
                        z = 0
                    #print x,y,z
                    t = getSecs(nmea[1])
                    if t is not None:
                        gz.SendCommand(' '.join(('gz tellObject',targets[src],'setPositionOrientationTime',str(x),str(y),str(-z),str(h[src]),'0.0 0.0',str(t))))
            if type == 'HDG':
                #print nmea
                h[src] = float(nmea[1])
                #print h

    # gz tellObject gps setPositionOrientationTime x y z h p r t