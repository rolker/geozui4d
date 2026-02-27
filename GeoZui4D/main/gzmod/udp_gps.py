#!/usr/bin/env python
# gz
# udp_gps
# reads gps data

import sys
import gz4d
import datetime
import calendar
import socket
import select
import math

def getSecs(tstr,dstr):
    if dstr is not None:
        day = int(dstr[0:2])
        month = int(dstr[2:4])
        year = int(dstr[4:])+2000
        hour = int(tstr[0:2])
        minute = int(tstr[2:4])
        sec = float(tstr[4:])
        isec = int(sec)
        fsec = sec-isec
        t = datetime.datetime(year,month,day,hour,minute,isec)
        return calendar.timegm(t.utctimetuple())+fsec

print 'Starting gps.py'

s = []
b = []

for i in range(3):
    s.append(socket.socket(socket.AF_INET, socket.SOCK_DGRAM))
    b.append('')
    s[i].bind (('',4000+i))
    s[i].settimeout(0.1)


zone = None
date = None

gz = gz4d.Connection()
gz.Connect()

result = gz.SendCommand('gz ping')
print result[1]

gps = socket.socket(type=socket.SOCK_DGRAM)
gps.bind(('',4000))

lastx = None
lasty = None

while True:
    line = gps.recv(256)
    nmea = line.strip().split(',')
    type = nmea[0][-3:]
    #print type
    if type == 'GGA':
        #print nmea
        lat = int(nmea[2][0:2])+float(nmea[2][2:])/60.0
        if nmea[3] == 'S':
            lat = -lat
        lon = int(nmea[4][0:3])+float(nmea[4][3:])/60.0
        if nmea[5] == 'W':
            lon = -lon
        #print lat,lon
        if zone is None:
            result = gz.SendCommand('gz projection init; gz projection longToUTM '+str(lon))
            zone = result[1]
            print zone
        result = gz.SendCommand(' '.join(('gz projection apply',zone,str(lon),str(lat))))
        #print result[1]
        en = result[1].split()
        x = float(en[0])
        y = float(en[1])
        lastx = x
        lasty = y
        z = nmea[9]
        t = getSecs(nmea[1],date)
        gz.SendCommand(' '.join(('gz tellObject NancyFoster setPositionOrientationTime',str(x),str(y),'0.0',str(h),'0.0 0.0',str(t))))
    if type == 'RMC':
        #print nmea
        date = nmea[9]
    if type == 'HDT':
        #print nmea
        h = float(nmea[1])#-90.0
        #print h
