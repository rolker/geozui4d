#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import rtNMEAUI
import wx
import socket
import datetime

def checksumStr(data):
        """
        Take a NMEA 0183 string and compute the checksum.
        @param data: NMEA message.  Leading $/! and training checksum are optional
        @type data: str
        @return: hexidecimal value
        @rtype: str

        Checksum is calculated by xor'ing everything between $ or ! and the *

        >>> checksumStr("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
        '09'
        >>> checksumStr("AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0")
        '09'
        """

        # FIX: strip off new line at the end too
        if data[0]=='!' or data[0]=='$': data = data[1:]
        if data[-1]=='*': data = data[:-1]
        if data[-3]=='*': data = data[:-3]
        # FIX: rename sum to not shadown builting function
        sum=0
        for c in data:
                sum = sum ^ ord(c)
        sumHex = "%x" % sum
        if len(sumHex)==1: sumHex = '0'+sumHex
        return sumHex.upper()

######################################################################
# common variables
import re
nmeaChecksumRegExStr = r"""\,[0-9]\*[0-9A-F][0-9A-F]"""
nmeaChecksumRE = re.compile(nmeaChecksumRegExStr)

def isChecksumValid(nmeaStr, allowTailData=True):
        """Return True if the string checks out with the checksum

        @param allowTailData: Permit handing of Coast Guard format with data after the checksum
        @param data: NMEA message.  Leading $/! are optional
        @type data: str
        @return: True if the checksum matches
        @rtype: bool

        >>> isChecksumValid("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
        True

        Corrupted:

        >>> isChecksumValid("!AIVDM,11,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
        False
        """

        if allowTailData:
                match = nmeaChecksumRE.search(nmeaStr)
                if not match:
                        return False
                nmeaStr = nmeaStr[:match.end()]
                #if checksum.upper()==checksumStr(nmeaStr[match.end()


        if nmeaStr[-3]!='*':
                print 'FIX: warning... bad nmea string'
                return False  # Bad string without proper checksum
        checksum=nmeaStr[-2:]
        if checksum.upper()==checksumStr(nmeaStr).upper(): return True
        return False


class rtNMEAFrame(rtNMEAUI.rtNMEAFrameBase):
    def __init__(self, *args, **kwds):
        rtNMEAUI.rtNMEAFrameBase.__init__(self, *args, **kwds)
        self.timer = wx.PyTimer(self.OnTimer)
        self.timer.Start(500,False)
        self.inSocket = None
        self.outSocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        self.gzport = 4010
        self.buf = ''
        self.nav = {}
        self.epoch = datetime.date(1970,1,1)

    def OnPort(self, evt):
        port = int(self.portTextCtrl.GetValue())
        self.inSocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        try:
            self.inSocket.bind(('',port))
            self.inSocket.settimeout(0.1)
        except socket.error:
            self.inSocket = None

    def OnGZPort(self, evt):
        self.gzport = int(self.gzportTextCtrl.GetValue())

    def OnTimer(self):
        if self.inSocket is not None:
            while True:
                try:
                    msg = self.inSocket.recv(2048)
                except socket.timeout:
                    break
                #self.NMEATextCtrl.AppendText(msg)
                self.ProcessPacket(msg)
                wx.Yield()

    def ProcessPacket(self,packet):
        self.buf = ''.join((self.buf,packet))

        while '*' in self.buf and len(self.buf.split('*',1)[1]) >= 2:
            nmea = '*'.join((self.buf.split('*',1)[0],self.buf.split('*',1)[1][0:2]))
            self.buf = self.buf.split('*',1)[1]
            if '$' in nmea:
                nmea = '$'+nmea.rsplit('$',1)[1]

                if nmea and not isChecksumValid(nmea,False):
                    print 'invalid:', nmea
                if nmea and isChecksumValid(nmea,False):
                    nmea = nmea.split('*')[0].split(',')
                    nt = nmea[0][3:6]
                    #print nt
                    try:
                        if nt in ('GGA','ZDA','SHR'): # all start with time 
                            self.nav['time'] = int(nmea[1][:2])*3600 + int(nmea[1][2:4])*60 + float(nmea[1][4:]) # s since midnight
                            if self.nav['time'] < 0:
                                print 'time:',self.nav['time']
                        if nt == 'HDT':
                            self.nav['heading'] = float(nmea[1])
                        #if nt == 'VTG':
                            #self.nav['cmg'] = float(nmea[1])
                            #self.nav['speed'] = float(nmea[7])/3.6 #km/h -> m/s
                        if nt == 'ZDA':
                            self.nav['day'] = (datetime.date(int(nmea[4]),int(nmea[3]),int(nmea[2])) -self.epoch).days*3600*24
                            if self.nav['day'] < 0:
                                print 'day:',self.nav['day']
                        if nt == 'SHR':
                            if int(nmea[11]) == 1:
                                self.nav['heading'] = float(nmea[2])
                                self.nav['roll'] = float(nmea[4])
                                self.nav['pitch'] = float(nmea[5])
                                self.nav['heave'] = float(nmea[6])
                                #print 'ypr heave',self.nav['heading']
                        if nt == 'GGA':
                            if int(nmea[6]) != 0: # fix is valid
                                lat = int(nmea[2][0:2]) + float(nmea[2][2:])/60.0
                                if nmea[3] in ('S','s'):
                                    lat = -lat
                                lon = int(nmea[4][0:3]) + float(nmea[4][3:])/60.0
                                if nmea[5] in ('W','w'):
                                    lon = -lon
                                self.nav['lat'] = lat
                                self.nav['lon'] = lon
                                #print 'll',lat,lon
                                position = ''
                                orientation = ''
                                time = ''
                                try:
                                    position = ''.join(('position="',str(self.nav['lon']),' ',str(self.nav['lat']),' ',str(self.nav['heave']),'"'))
                                except KeyError:
                                    try:
                                        position = ''.join(('position="',str(self.nav['lon']),' ',str(self.nav['lat']),' 0.0"'))
                                    except KeyError:
                                        pass
                                try:
                                    orientation = ''.join(('orientation="'+str(self.nav['roll'])+' '+str(self.nav['pitch'])+' '+str(self.nav['heading'])+'"'))
                                except KeyError:
                                    try:
                                        orientation = ''.join(('orientation="0.0 0.0 '+str(self.nav['heading'])+'"'))
                                    except KeyError:
                                        pass
                                try:
                                    time=''.join(('time="',str(self.nav['day']+self.nav['time']),'"'))
                                except KeyError:
                                    pass
    
                                if position and orientation and time:
                                    xml = ' '.join(('<GZ><Position',time,position,orientation,'/></GZ>\n'))
                                    self.NMEATextCtrl.AppendText(xml)
                                    self.outSocket.sendto(xml,(('',self.gzport)))
                                else:
                                    print 'Skipped position:',position,'orientation:',orientation,'time:',time
                    except (ValueError,IndexError):
                        print 'Skipping problem line:',l.strip()

class rtNMEAApp(wx.App):

    def OnInit(self):
        self.frame = rtNMEAFrame(None)
        self.frame.Show()
        self.SetTopWindow(self.frame)
        return True
