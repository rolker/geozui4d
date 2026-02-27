#!/usr/bin/env python
# -*- coding: utf-8 -*-

import serial
import time
import struct
import threading

class InvalidDevice(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value


class Fastrak:
    def __init__(self, port=None):
        self.pt = None
        if port is None:
            port = 0
        self.fastrak = serial.Serial(port,115200,timeout=1)

        self.fastrak.write('\n') # send linefeed to flush input buffer, in case something was left over
        self.Continuous(False)
        time.sleep(.1)
        self.fastrak.flushInput()
        self.state = []

        for i in range(4):
            self.state.append({'position':None,'orientation':None})

        status = self.Status()
        if len(status):
            if status[21:53] == " "*32:
                print 'isotrak'
                self.pt = threading.Thread(target=self.IsotrackPositionThread)
            else:
                print 'fastrak'
                self.pt = threading.Thread(target=self.PositionThread)
            self.pt.start()
        else:
            raise InvalidDevice(status)

    def __del__(self):
        self.Stop()

    def Stop(self):
        if self.pt is not None:
            self.done = True
            self.pt.join()
            self.pt = None


    def SetStationValues(self,cmd,station,newValues):
        self.SetValues(cmd,[station] + list(newValues))

    def SetValues(self,cmd,newValues):
        cmdstr = cmd + str(newValues[0])
        for v in newValues[1:]:
            cmdstr += ','+str(v)
        self.fastrak.write(cmdstr + '\r\n')

    def ReadLine(self):
        return self.fastrak.readline()

    def Read(self,size):
        return self.fastrak.read(size)

    def GetValues(self,cmd,station,size=7):
        cmdstr = cmd + str(station) + '\r\n'
        self.fastrak.write(cmdstr)
        result = self.fastrak.readline()
        ret = [cmd]
        ret.append(int(result[1]))
        i = 3
        while i+size < len(result):
            ret.append(result[i:i+size])
            i += size
        return ret

    def Query(self,cmd):
        self.fastrak.write(cmd) 
        return self.fastrak.readline()

    def Command(self,cmd):
        self.fastrak.write(cmd)

    def BoolCmd(self,val,t,f):
        if val:
            self.Command(t)
        else:
            self.Command(f)

    def Binary(self,mode=True):
        self.BoolCmd(mode,'f','F')

    def Continuous(self,mode=True):
        self.BoolCmd(mode,'C','c')

    def Metric(self,mode=True):
        self.BoolCmd(mode,'u','U')

    def Position(self):
        return self.Query('P')

    def Status(self):
        return self.Query('S')

    def PositionThread(self):
        self.Binary()
        self.Metric()
        self.Command('x5\r\n')
        for i in range(1,5):
            self.SetStationValues('O',i,(52,61,1))
            self.SetStationValues('H',i,(0,1,0))

        self.Continuous()
        synced = False
        tries = 0
        while not synced and tries < 100:
            print 'sync'
            tries += 1
            p = self.Read(1)
            if p[0] == '0':
                p += self.Read(32)
                if p[-2:] == '\r\n':
                    synced = True
        if not synced:
            return
        self.done = False
        while not self.done:
            p = self.Read(33)
            if p[0] == '0':
                try:
                    station = int(p[1])-1
                    data = struct.unpack('<7f',p[3:31])
                    self.state[station]['position'] = data[:3]
                    self.state[station]['orientation'] = data[3:]
                except ValueError:
                    pass

    def IsotrackPositionThread(self):
        self.Binary()
        self.Metric()
        self.SetValues('O',(2,11))
        for i in range(1,3):
            self.SetStationValues('H',i,(0,1,0))

        self.Continuous()

        self.done = False

        data = []
        while not self.done:
            p = self.Read(1)
            if ord(p[0]) > 127:
                if len(data) == 20:
                    decode = ""
                    block = 0
                    byte = 0

                    done = False
                    while not done:
                        overflow = block*8+7
                        if overflow >= len(data):
                            overflow = len(data)-1
                        decode += chr((ord(data[block*8+byte])&0x7f)
                        ^ ( ((ord(data[overflow])>>byte)&0x01)<<7))
                        byte += 1
                        if byte >= 7:
                            block += 1
                            byte = 0
                        if block*8+byte >= len(data) - 1:
                            station = int(decode[1]) -1
                            ints = struct.unpack('<7h',decode[3:])
                            self.state[station]['position'] = []
                            for i in range(3):
                                self.state[station]['position'].append(ints[i]*0.005075838)
                            self.state[station]['orientation'] = []
                            for i in range(3,7):
                                self.state[station]['orientation'].append(ints[i]/32767.0)
                            done = True

                data = []
            data += p


if __name__ == '__main__':
    import sys
    p = None

    if len(sys.argv) > 1:
        p = sys.argv[1]

    f = Fastrak(p)

    try:
        while True:
            if f.state[0]['position'] is not None and f.state[0]['orientation'] is not None:
                position_str = '0:'
                quat_str = 'q:'
                for i in f.state[0]['position']:
                    position_str += '\t'+str(int(i*100)/100.0)
                for i in f.state[0]['orientation']:
                    quat_str += '\t'+str(int(i*100)/100.0)
                a = f.state[0]['orientation'][0]
                b = f.state[0]['orientation'][1]
                c = f.state[0]['orientation'][2]
                d = f.state[0]['orientation'][3]
                rot = ((a*a+b*b-c*c-d*d,2*b*c-2*a*d,2*a*c+2*b*d),
                        (2*a*d+2*b*c,a*a-b*b+c*c-d*d,2*c*d-2*a*b),
                        (2*b*d-2*a*c,2*a*b+2*c*d,a*a-b*b-c*c+d*d))
                rvec = (-rot[0][2],-rot[1][2],-rot[2][2])
                print position_str,rvec
            time.sleep(0.05)
    except KeyboardInterrupt:
        print 'exiting'
        f.Stop()
