#!/usr/bin/env python
# gz
# sendAIS
# Send AIS data from a UDP port to GZ

import socket
import ais
import gz4d
import calendar
import datetime

ais_in = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ais_in.bind(('',4001))

gz = gz4d.Connection()
gz.Connect()

ais_id = gz.SendCommand('gz newobject ObjectCollection AIS')[1]
print ais_id

bv = {'A':None,'B':None}
msgType = {'A':None,'B':None}

ships = {}

def fixName(n):
    return n.split('@',1)[0].strip()

while True:
    msg = ais_in.recv(1500)

    parts = msg.split(',')
    if parts[0] == '!AIVDM':
        msgPayload = parts[5]
        channel = parts[4]
    
        if parts[2] == '1': # we expect the first part of a message
            bv[channel] = ais.binary.ais6tobitvec(msgPayload)
            msgType[channel] = msgPayload[0]
        else:
            if bv[channel] is not None:
                bv[channel] += ais.binary.ais6tobitvec(msgPayload)
    
        if parts[1] == parts[2] and bv[channel] is not None:
            if msgType[channel] in ('1','2','3'):
                msgDict = ais.ais_msg_1.decode(bv[channel])
                uid = msgDict['UserID']

                if not ships.has_key(uid):
                    ships[uid] = {}
                if not ships[uid].has_key('gz'):
                    ships[uid]['gz'] = gz.SendCommand('gz tellObject '+ais_id+' newobject BasicVessel '+str(uid))[1]
                    gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' projection Geographic')
                    gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' wireframe on')
                gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' setPositionOrientationTime '+str( msgDict['Position_longitude']) +' '+str(msgDict['Position_latitude'])+' 0.0 '+str(msgDict['TrueHeading'])+' 0.0 0.0 '+str(calendar.timegm(datetime.datetime.utcnow().timetuple())))

            if msgType[channel] == '5':
                msgDict = ais.ais_msg_5.decode(bv[channel])
                uid = msgDict['UserID']
                if not ships.has_key(uid):
                    ships[uid] = {}
                if not ships[uid].has_key('info'):
                    ships[uid]['info'] = None
                if ships[uid].has_key('gz'):
                    if ships[uid]['info'] != msgDict:
                        ships[uid]['info'] = msgDict
                        gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' aisName '+ fixName(msgDict['name']))
                        gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' destination '+ fixName(msgDict['destination']))
                        gz.SendCommand('gz tellObject '+ ships[uid]['gz'] + ' callsign '+ fixName(msgDict['callsign']))



