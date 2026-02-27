#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import gz4d
import datetime
import calendar
import math

nmea_files = []
model = None
altitude = False
speedUnits = None
speedFactor = 1.0
pathColor = ""
pathOn = ' path="on"'
strict = True

minppm = None

for a in sys.argv[1:]:
    if '=' in a:
        key,val = a.split('=',1)
        if key in ('a','altitude'):
            if val in ('1','true','True'):
                altitude = True
        if key == 'strict':
            if val in ('0','false','False'):
                strict = False
        if key == 'path':
            if val in ('0','false','False'):
		pathOn = ''
        if key in ('s','speed'):
            speedUnits = val
        if key == 'model':
            model = val
        if key == 'minppm':
            minppm = val
        if key == 'pathColor':
            pathColor = ' pathColor="'+val+'"'
    else:
        nmea_files.append(a)

if speedUnits in ('mph',):
    speedFactor = 1.15
if speedUnits in ('kph',):
    speedFactor = 1.852
if speedUnits in ('m/s',):
    speedFactor = 0.514444444


sstr = ''
if speedUnits is not None:
    sstr = ' speed_units="'+speedUnits+'"'

print '<GZ>\n\t<Object type="Vessel"'+pathOn+pathColor+' projection="Geographic"'+sstr+'>'

if model is not None:
    mstr = '\t\t<Object filename="'+model+'"'
    if minppm is not None:
        mstr += ' minimumPixelsPerMeter="'+minppm+'"'
    mstr += '/>'
    print mstr


zda = None
alt = '0.1'
sog = '0.0'
using_rmc = False
heading = None
heading_str = ''

last_time = None

for f in nmea_files:
    for l in file(f):
        try:
            s = gz4d.data.nmea.Sentence(l,strict)
            if s.IsType('??ZDA'):
                zda = gz4d.data.nmea.ZDA(s)
            if s.IsType('??RMC'):
                using_rmc = True
                rmc = gz4d.data.nmea.RMC(s)
                t = calendar.timegm(datetime.datetime(rmc.year,rmc.month,rmc.day,rmc.hour,rmc.minute).utctimetuple())+rmc.second
                if last_time is not None and t < last_time:
                    print 'crap!'
                last_time = t
                sog = str(rmc.sog*speedFactor)
                if speedUnits is not None:
                    spd = ' speed="'+sog+'"'
                else:
                    spd = ''
                print '\t\t<Position time="'+str(t)+'" position="'+str(rmc.longitude)+' '+str(rmc.latitude)+' '+alt+'"'+spd+heading_str+'/>'
    
            if s.IsType('??HDG'):
                hdg = gz4d.data.nmea.HDG(s)
                heading = hdg.magnetic_heading+hdg.magnetic_declination
    
            if s.IsType('??HDT'):
                hdt = gz4d.data.nmea.HDT(s)
                heading = hdt.heading

            if s.IsType('??GGA') and not using_rmc:
                gga = gz4d.data.nmea.GGA(s)
                if zda is not None:
                    t = calendar.timegm(datetime.datetime(zda.year,zda.month,zda.day,gga.hour,gga.minute).utctimetuple())+gga.second
                    if altitude:
                        alt = str(gga.altitude)
                    if speedUnits is not None:
                        spd = ' speed="'+sog+'"'
                    else:
                        spd = ''
                    print '\t\t<Position time="'+str(t)+'" position="'+str(gga.longitude)+' '+str(gga.latitude)+' '+alt+'"'+spd+heading_str+'/>'
            if heading is not None and not math.isnan(heading):
                heading_str = ' heading="'+str(heading)+'"'
            else:
                heading_str = ''
        except ValueError:
            pass

print '\t</Object>\n</GZ>'
