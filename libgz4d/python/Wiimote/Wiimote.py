#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import bluetooth
import threading
import time

class Wiimote:
    cmds = {'lights':(0x52,0x11),
        'sendMotion':(0x52,0x12,0x00,0x37),
        'sendCalibration':(0x52,0x17,0x00,0x00,0x00,0x16,0x00,0x08),
        'enableNunchuk':(0x52,0x16,0xa4,0x00,0x40,0x01,0x00),
        'getNunchukCalibration':(0x52,0x17,0x04,0xA4,0x00,0x20,0x00,0x10),
        'rumble':(0x52,0x13)}

    def __init__(self):
        self.ctrl_socket = None
        self.data_socket = None
        self.quitLock = threading.Lock()
        self.dataLock = threading.Lock()
        self.buttonEvents = []
        self.quitSignal = False
        self.dataThread = None
        self.wiimoteButtons = 0

    def find(self,possible=None):
        if self.dataThread is None:
            print 'Trying default...'
            if self.connect(possible):
                return True
            print 'Looking for any Wiimote...'
            devices = bluetooth.discover_devices(lookup_names = True)
            for d in devices:
                if d[1] == 'Nintendo RVL-CNT-01':
                    return self.connect(d[0])
        return False

    def connect(self,addr=None):
        if addr is None:
            return False
        print 'Connecting to',addr
        try:
            self.data_socket = bluetooth.BluetoothSocket( bluetooth.L2CAP )
            self.data_socket.settimeout(1.5)
            self.data_socket.connect((addr,19))
            self.data_socket.settimeout(.5)
            self.ctrl_socket = bluetooth.BluetoothSocket( bluetooth.L2CAP )
            self.ctrl_socket.connect((addr,17))
        except bluetooth.BluetoothError:
            print 'Failed...'
            self.data_socket = None
            self.ctrl_socket = None
            return False
        self.quitSignal=False
        self.dataThread = threading.Thread(target=Wiimote.processData,name='Wiimote-'+addr,args=(self,))
        print 'Launching thread'
        self.dataThread.start()
        return True

    def sendCommand(self,command,value=None):
        if self.ctrl_socket is not None:
            cmd = ''
            for c in Wiimote.cmds[command]:
                cmd += chr(c)
            if value is not None:
                cmd += chr(value)
            self.ctrl_socket.send(cmd)
            return True
        return False

    def setLights(self,value):
        return self.sendCommand('lights',value*16)

    def setSendMotion(self):
        return self.sendCommand('sendMotion')

    def getCalibration(self):
        return self.sendCommand('sendCalibration')

    def getButtonEvent(self):
        ret = None
        self.dataLock.acquire()
        if len(self.buttonEvents) > 0:
            ret = self.buttonEvents.pop(0)
        self.dataLock.release()
        return ret

    def stopDataThread(self):
        if self.dataThread is not None:
            self.quitLock.acquire()
            self.quitSignal = True
            self.quitLock.release()
            self.dataThread.join()
            self.dataThread = None
            return True
        return False

    def close(self):
        self.stopDataThread()
        if self.ctrl_socket is not None:
            self.ctrl_socket.close()
        if self.data_socket is not None:
            self.data_socket.close()

    def processData(self):
        done = False
        while not done:
            self.quitLock.acquire()
            done = self.quitSignal
            self.quitLock.release()
            if not done:
                try:
                    data = self.data_socket.recv(1024)
                    self.decode(data)
                except bluetooth.BluetoothError:
                    pass
        print 'Exiting processData thread...'

    def decode(self,data):
        out = []
        for i in range(len(data)):
            out.append(str(ord(data[i])))
        print ', '.join(out)
        print 0xa1
        if ord(data[0]) == 0xa1:
            if ord(data[1]) in (0x30,0x31,0x37):
                buttons = ord(data[3])*256+ord(data[2])
                self.buttonEvents.append((self.wiimoteButtons^buttons,bool((self.wiimoteButtons^buttons)&buttons)))
                self.wiimoteButtons = buttons

if __name__ == '__main__':
    w = Wiimote()
    print 'Looking...'
    cached = '00:1A:E9:22:87:DA'
    if(w.find(cached)):
        print 'Connected!'
        for i in range(16):
            w.setLights(i)
            time.sleep(.75)
        print 'Closing connection to Wiimote...'
        w.close()
    print 'Done!'
