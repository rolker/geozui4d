#!/usr/bin/env python
# gz
# nosb
# Cycle through National Ocean Science Bowl scoring billboards

import gz4d
import time
import math
import sys

gz = gz4d.Connection()
gz.Connect()

#gz.SendCommand('global mainWindow; $mainWindow useHeightExaggeration 6')

gz.SendCommand('gz newobject Kinect team_kinect 1249473.62 4340000 -4000.98')
gz.SendCommand('gz posable set Object.team_kinect s 5000')

teams = []

ret = gz.SendCommand('gz posable list .Posable.Object')
for o in ret[1].split():
    po = o.split('.')[-1]
    if po.startswith('team'):
        teams.append({'name':po})

#teams = teams[-3:]

for t in teams:
    ret = gz.SendCommand('gz posable get Object.'+t['name']+' xyzhpr')
    t['x'],t['y'],t['z'],t['h'],t['p'],t['r'] = ret[1].split()
    ret = gz.SendCommand('gz tellObject '+t['name']+' ignoreHeightExaggeration')
    ret = gz.SendCommand('gz tellObject '+t['name']+' hide')
    if t['name'] == 'team_kinect':
        t['y'] = str(float(t['y'])+ 5000.0)

baseScale = 30000
zoomOutTime = 1.75
transitTime = 2.0
dwellTime = 6.0
headingSwing = 10.0


while True:
    
    for t in teams:
        gz.SendCommand('global interactivityHint; set interactivityHint .1')

        startTime = time.time()
        now = startTime
        if t == teams[0]:
            hFrom = float(teams[-1]['h'])+headingSwing
            xFrom = float(teams[-1]['x'])
            yFrom = float(teams[-1]['y'])
            zFrom = float(teams[-1]['z'])
            pFrom = float(teams[-1]['p'])
        else:
            hFrom = float(teams[teams.index(t)-1]['h'])+headingSwing
            xFrom = float(teams[teams.index(t)-1]['x'])
            yFrom = float(teams[teams.index(t)-1]['y'])
            zFrom = float(teams[teams.index(t)-1]['z'])
            pFrom = float(teams[teams.index(t)-1]['p'])
        hTo = float(t['h'])-headingSwing
        xTo = float(t['x'])
        yTo = float(t['y'])
        zTo = float(t['z'])
        pTo = float(t['p'])

        hDelta = hTo-hFrom
        xDelta = xTo-xFrom
        yDelta = yTo-yFrom
        zDelta = zTo-zFrom
        pDelta = pTo-pFrom

        if hDelta > 180.0:
            hDelta -= 360.0
        if hDelta < -180.0:
            hDelta += 360.0

        while (now < startTime+transitTime):
            p = (now-startTime)/transitTime
            hNow = hFrom+hDelta*p
            xNow = xFrom+xDelta*p
            yNow = yFrom+yDelta*p
            zNow = zFrom+zDelta*p
            pNow = pFrom+pDelta*p

            gz.SendCommand('gz posable set Window.main xyzhp '+str(xNow)+' '+str(yNow)+' '+str(zNow)+' '+str(hNow)+' '+str(pNow))
            now = time.time()

        gz.SendCommand('gz posable set Window.main xyzhp '+str(xTo)+' '+str(yTo)+' '+str(zTo)+' '+str(hTo)+' '+str(pTo))

        gz.SendCommand('gz tellObject '+t['name']+' show')

        startTime = time.time()
        now = startTime
        while now < startTime+zoomOutTime:
            gz.SendCommand('gz posable set Window.main S '+str(baseScale * pow(10,zoomOutTime-(now-startTime))))
            now = time.time()
        gz.SendCommand('gz posable set Window.main S '+str(baseScale))

        gz.SendCommand('global interactivityHint; set interactivityHint 0')
        gz.SendCommand('gz tellObject '+t['name']+' refresh')

        #time.sleep(dwellTime)

        startTime = time.time()
        now = startTime
        while now < startTime+dwellTime:
            gz.SendCommand('gz posable set Window.main h '+str(hTo+ (2*headingSwing*(now-startTime)/dwellTime) ))
            now = time.time()

        startTime = time.time()
        now = startTime
        while now < startTime+zoomOutTime:
            gz.SendCommand('gz posable set Window.main S '+str(baseScale * pow(10,now-startTime)))
            now = time.time()
        gz.SendCommand('gz posable set Window.main S '+str(baseScale * pow(10,zoomOutTime)))
        gz.SendCommand('gz tellObject '+t['name']+' hide')
        time.sleep(.25)
        

