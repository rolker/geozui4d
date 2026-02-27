#!/usr/bin/env python
# gz
# rtEK500
# Capture and display in real time data broadcast in EK500 format.

import sys
import gz4d
import datetime
import calendar
import socket
import struct
import math

class EK500:
    def __init__(self,port=3000):
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.sock.bind(('',port))

        self.gz = gz4d.Connection()
        self.gz.Connect()

        self.time = datetime.datetime.utcnow()
        self.positions = []
        self.targets = []
        self.targetQlength = len(self.targets)
        self.zone = None

        self.channelID = {}

    def getData(self):
        data = self.sock.recv(2048)
        msgID,msg = data.split(',',1)
        try:
            channelNum = int(msgID[1])
            msgID = msgID[0:1]
        except ValueError:
            channelNum = None
        try:
            EK500.handlers[msgID](self,channelNum,msg)
        except KeyError:
            print msgID,'of size',len(msg),'skipped.'
        self.checkQueues()

    def checkQueues(self):
        oldTargets = []
        for t in self.targets:
            #print 'target:',t
            p = self.getPosition(t[1])
            if p is not None:
                if not t[0] in self.channelID.keys():
                    res = self.gz.SendCommand('gz newobject EK500 EK60_channel'+str(t[0]))
                    self.channelID[t[0]] = res[1]
                gzcmd = ' '.join(('gz tellObject',self.channelID[t[0]],'addTarget',str(t[1]),str(p[0]),str(p[1]),str(t[2]),str(t[3])))
                print gzcmd
                self.gz.SendCommand(gzcmd)
                oldTargets.append(t)
        for t in oldTargets:
            self.targets.remove(t)

    def getPosition(self,time):
        if len(self.positions) >= 2:
            if time >= self.positions[0][0] and time <= self.positions[-1][0]:
                pos1 = None
                pos2 = None
                for i in range(len(self.positions)):
                    j = -(i+1)
                    if self.positions[j][0] >= time:
                        pos2 = self.positions[j]
                    else:
                        pos1 = self.positions[j]
                        break
                print time,pos1,pos2
                r = (time-pos1[0])/(pos2[0]-pos1[0])
                course = math.degrees(math.atan2(pos2[1]-pos1[1],pos2[2]-pos1[2]))
                if course < 0.0:
                    course += 360.0
                x = pos1[1]*r+pos2[1]*(1.0-r)
                y = pos1[2]*r+pos2[2]*(1.0-r)
                print 'r:',r,'pos:',x,y,'course:',course
                return (x,y,course)

    def processB(self,channelNum,data):
        '''sample angle data'''
        t,raw = data.split(',',1)
        #print msgNum,'B'

    def processD(self,channelNum,data):
        '''Depth'''
        t,raw = data.split(',',1)
        depth,backScatter,transducerNum = struct.unpack('<2fL',raw[0:12])
        #print msgNum,transducerNum,"depth:",depth,'bs:',backScatter

    def processE(self,channelNum,data):
        '''echotrace'''
        t,raw = data.split(',',1)
        eNum = struct.unpack('<L',raw[0:4])[0]
        if eNum:
            print 'E:',t,channelNum,
            #print msgNum,'Traces:',
            for i in range(eNum):
                values = struct.unpack('<5f',raw[4+i*20:4+(i+1)*20])
                self.targets.append((channelNum,self.getSecs(t),-values[0],values[1],values[3],values[4]))
                print self.targets[-1],
            print

    def processGL(self,channelNum,data):
        parts = data.split(',')
        lat = int(parts[1][0:2])+float(parts[1][2:])/60.0
        if parts[2] == 'S':
            lat = -lat
        lon = int(parts[3][0:3])+float(parts[3][3:])/60.0
        if parts[4] == 'W':
            lon = -lon
        #print lat,lon
        if self.zone is None:
            result = self.gz.SendCommand('gz projection init; gz projection longToUTM '+str(lon))
            self.zone = result[1]
        result = self.gz.SendCommand(' '.join(('gz projection apply',self.zone,str(lon),str(lat))))
        en = result[1].split()
        x = float(en[0])
        y = float(en[1])
        self.positions.append((self.getSecs(parts[0]),x,y))
        #print 'GL:',self.positions[-1]


    def processP(self,channelNum,data):
        '''sample TS data'''
        t,raw = data.split(',',1)
        print channelNum,'P'

    def processQ(self,channelNum,data):
        '''Echogram '''
        t,raw = data.split(',',1)
        #print 'Q: ',t

    def processPE(self,channelNum,data):
        '''parameters'''
        t,raw = data.split(',',1)
        print channelNum,'PE',raw

    def processV(self,channelNum,data):
        '''sample Sv data'''
        t,raw = data.split(',',1)
        print channelNum,'V'

    def processW(self,channelNum,data):
        '''sample power data'''
        t,raw = data.split(',',1)
        #print msgNum,'W'

    def getSecs(self,time):
        return calendar.timegm(self.time.replace(hour=int(time[0:2]), minute = int(time[2:4]), second = int(time[4:6])).utctimetuple())+(int(time[6:8])/100.0)

    handlers = {'B' :processB,
                'D' :processD,
                'E' :processE,
                'GL':processGL,
                'P' :processP,
                'Q' :processQ,
                'PE':processPE,
                'V' :processV,
                'W' :processW,
               }

if __name__ == '__main__':
    ek = EK500()
    while True:
        ek.getData()

