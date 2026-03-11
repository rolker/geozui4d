#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# gps_tcp
# Read gps from tcp connection

import sys
import gz4d
import datetime
import calendar
import socket


def getSecs(tstr,dstr=None,zda=None):
    if dstr is None and zda is None:
        return None
    if dstr is not None:
        day = int(dstr[0:2])
        month = int(dstr[2:4])
        year = int(dstr[4:])+2000
    if zda is not None:
        day = int(zda[0])
        month = int(zda[1])
        year = int(zda[2])
    hour = int(tstr[0:2])
    minute = int(tstr[2:4])
    sec = float(tstr[4:])
    isec = int(sec)
    fsec = sec-isec
    t = datetime.datetime(year,month,day,hour,minute,isec)
    return calendar.timegm(t.utctimetuple())+fsec


zone = None
date = None
zda = None
h='0.0'

gz = gz4d.Connection()
gz.Connect()

s = None

while True:
    if s is None:
        s = socket.socket()
        s.connect(('192.168.42.40',20002))
    data = s.recv(1024)
    if len(data) == 0:
        s.close()
        s = None
    else:
        lines = data.split()
        if len(lines) >0:
            for l in lines:
                nmea = l.strip().split(',')
                #print nmea
                if '*' in nmea[-1]:
                    type = nmea[0][-3:]
                    #print type
                    if type == 'GGA':
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
                        x = en[0]
                        y = en[1]
                        z = nmea[9]
                        print x,y,z
                        t = getSecs(nmea[1],date,zda)
                        if t is not None:
                            gz.SendCommand(' '.join(('gz tellObject auk setPositionOrientationTime',x,y,'0.0',h,'0.0 0.0',str(t))))
                    if type == 'RMC':
                        date = nmea[9]
                    if type == 'ZDA':
                        zda = (nmea[2],nmea[3],nmea[4])
                    if type == 'HDG':
                        h = nmea[1]
                        print h
                    if type == 'HDT':
                        h = nmea[1]
                        print h
