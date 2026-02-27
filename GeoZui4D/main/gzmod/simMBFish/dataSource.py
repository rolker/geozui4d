#!/usr/bin/env python

import gz4d
import xml.etree.ElementTree
import Scientific.IO.NetCDF
import struct
import array
import datetime
import calendar
import os
import socket
import time

class DataSource:
    def __init__(self,fname,manager):
        self.fname = fname
        self.startTime = None
        self.endTime = None
        self.manager = manager
        self.values = None
        self.gzObject = None
        self.groupSize = 1

    def Start(self,objType=None,objLabel=None):
        if self.values is not None:
            self.index = 0
            ret = self.manager.GzCommand('gz newobject '+objType+' '+objLabel)
            if ret is not None:
                self.gzObject = ret[1]
                self.socket = socket.socket(type=socket.SOCK_DGRAM)
                self.socket.bind(('',0))
                self.myport = self.socket.getsockname()[1]
                ret = self.TellObject('connect '+socket.gethostname()+' '+str(self.myport))
                if ret is not None and ret[0]:
                    self.remotePort = int(ret[1])

    def Idle(self,simNow):
        if self.values is not None:
            count = 0
            while self.index < len(self.values) and simNow >= self.GetTime(self.index) and count < self.groupSize:
                if self.gzObject is not None:
                    self.SendValue(self.index)
                self.index += 1
                count += 1
            print self.gzObject,count
            if self.index < len(self.values):
                return  max(self.GetTime(self.index)-simNow,0)

    def TellObject(self,msg):
        if self.gzObject:
            return self.manager.GzCommand(' '.join(('gz tellObject',self.gzObject,msg)))

    def SendPacket(self,packet):
        self.socket.sendto(packet,(self.manager.gzAddress,self.remotePort))

    
class PositionSource(DataSource):
    def __init__(self,fname,manager):
        DataSource.__init__(self,fname,manager)
        self.groupSize =15
        self.geographic = False
        self.vessel = None
        try:
            gzx = xml.etree.ElementTree.ElementTree(file=self.fname)
            gz = gzx.getroot()
            try:
                if gz.tag == 'GZ':
                    for o in gz.findall('Object'):
                        try:
                            if o.get('type') in ['Vessel','BasicVessel']:
                                try:
                                    self.geographic = (o.get('projection')=='Geographic')
                                except KeyError:
                                    self.geographic = False
                                self.values = []
                                for p in o.findall('Position'):
                                    self.values.append((float(p.get('time')),p.get('position'),p.get('orientation')))
                                self.startTime = self.values[0][0]
                                self.endTime = self.values[-1][0]
                                m = o.find('Object')
                                if m is not None:
                                    model = os.path.abspath(os.path.join(os.path.split(fname)[0],m.get('filename')))
                                    try:
                                        if self.manager.sources['model'].fname == '':
                                            self.manager.Init('model',model)
                                    except KeyError:
                                        self.manager.Init('model',model)
                                    
                                break
                        except KeyError:
                            pass
            except AttributeError:
                self.values = None
        except IOError:
            self.values = None

    def Start(self):
        DataSource.Start(self,'BasicVessel','vessel')
        self.TellObject('setRT')
        
        if self.geographic:
            self.TellObject('projection Geographic')
        try:
            self.TellObject('loadobject '+self.manager.sources['model'].fname)
        except KeyError:
            pass

    def GetTime(self,index):
        try:
            return self.values[index][0]
        except IndexError:
            return None


    def SendValue(self,index):
        orientation = self.values[index][2].split()
        self.TellObject(' '.join(('setPositionOrientationTime',self.values[index][1],orientation[2],orientation[1],orientation[0],str(self.values[index][0]))))
        if index == 0:
            self.TellObject('path visible on')

class NetCDFSource(DataSource):
    def __init__(self,fname,manager):
        DataSource.__init__(self,fname,manager)
        try:
            self.ncfile = Scientific.IO.NetCDF.NetCDFFile(self.fname,'r')
        except IOError:
            self.ncfile = None

class PointSource(NetCDFSource):
    def __init__(self,fname,manager):
        NetCDFSource.__init__(self,fname,manager)

        if self.ncfile is not None:
            try:
                self.baseTime = self.ncfile.variables['baseTime'][0]
                self.baseE = self.ncfile.variables['baseUtmEasting'][0]
                self.baseN = self.ncfile.variables['baseUtmNorthing'][0]
                self.zone = self.ncfile.variables['baseUtmZone'][0]
                self.values = self.ncfile.variables['targets']
                self.startTime = self.baseTime + self.Decode(self.values[0])[3]
                self.endTime = self.baseTime + self.Decode(self.values[-1])[3]
            except KeyError:
                self.values = None

    
    def Decode(self,target):
        return struct.unpack('4f4B',array.array('b',target).tostring())

    def GetTime(self,index):
        try:
            return self.baseTime + self.Decode(self.values[index])[3]
        except IndexError:
            return None

    def SendValue(self,index):
        data = self.Decode(self.values[index])
        time = self.baseTime + data[3]
        e = self.baseE + data[0]
        n = self.baseN + data[1]
        packet = struct.pack('3df4B',time,e,n,data[2],data[4],data[5],data[6],data[7])
        self.SendPacket(packet)


class BathymetrySource(PointSource):
    def __init__(self,fname,manager):
        PointSource.__init__(self,fname,manager)
        self.groupSize = 100
        
    def Start(self):
        DataSource.Start(self,'XYZStream','bathymetry')
        if self.gzObject is not None:
            self.TellObject('useAlternateData')
            self.TellObject('initialize 1.0 swa 2.0')


class TargetSource(PointSource):
    def __init__(self,fname,manager):
        PointSource.__init__(self,fname,manager)
        self.groupSize = 10

    def Start(self):
        DataSource.Start(self,'FishSchool','fish')


class RasterSource(NetCDFSource):
    def __init__(self,fname,manager):
        NetCDFSource.__init__(self,fname,manager)
        self.groupSize = 10
        if self.ncfile is not None:
            self.rows = self.ncfile.dimensions['row']
            self.cols = self.ncfile.dimensions['column']
            self.times = self.ncfile.variables['time']
            self.startTime = self.times[0]
            self.endTime = self.times[-1]

    def GetTime(self,index):
        try:
            return self.times[index]
        except IndexError:
            return None

    def SendValue(self,index):
        for i in range(self.rows):
            packet = struct.pack('<d',self.times[index])+self.code+struct.pack('<3H',i,self.rows,self.cols)+self.values[index,i].tostring()
            self.SendPacket(packet)

class GridSource(RasterSource):
    def __init__(self,fname,manager):
        RasterSource.__init__(self,fname,manager)
        self.code = '2f'
        self.imageSet = False
        if self.ncfile is not None:
            self.values = self.ncfile.variables['grids']

    def Start(self):
        DataSource.Start(self,'DistortionGrid','grid')
        if self.gzObject is not None:
            self.TellObject('setTexture '+self.manager.sources['image'].gzObject)
            self.manager.GzCommand('gz posable set Object.vessel xyzhpr 0 0 0 0 0 0; gz posable couple Object.grid to Object.vessel Htld; gz posable set Object.grid yz 10 3')

class ImageSource(RasterSource):
    def __init__(self,fname,manager):
        RasterSource.__init__(self,fname,manager)
        self.code = '4B'
        self.groupSize = 1
        if self.ncfile is not None:
            self.values = self.ncfile.variables['images']

    def Start(self):
        DataSource.Start(self,'ImageStream','watercolumn')




class SourceManager:
    sourceMap = {'positions':PositionSource,'bathymetry':BathymetrySource,'targets':TargetSource,'model':DataSource,'image':ImageSource,'grid':GridSource}
    def __init__(self):
        self.gz = None
        self.geographic = False
        self.sources = {}
        self.startTime = None
        self.endTime = None
        self.gzAddress = 'localhost'

    def Connect(self,address,port):
        self.gzAddress = address
        self.gz = gz4d.Connection()
        self.gz.Connect(':'.join((address,port)))

    def GzCommand(self,cmd):
        if self.gz is not None:
            return self.gz.SendCommand(cmd)

    def Init(self,source,fname):
        self.sources[source] = SourceManager.sourceMap[source](fname,self)
        if self.startTime is None or (self.sources[source].startTime is not None and self.sources[source].startTime < self.startTime):
            self.startTime = self.sources[source].startTime
        if self.endTime is None or (self.sources[source].endTime is not None and self.sources[source].endTime > self.endTime):
            self.endTime = self.sources[source].endTime

    def Start(self,fnames):
        for s,fn in fnames.items():
            try:
                if self.sources[s].fname != fn:
                    self.Init(s,fn)
            except KeyError:
                self.Init(s,fn)

        for s in ('positions','bathymetry','targets','model','image','grid'):
            try:
                self.sources[s].Start()
            except KeyError:
                pass    

        self.simTimeOffset = self.startTime - self.SysTime()

    def GetProgress(self):
        self.simNow = self.SysTime() + self.simTimeOffset
        return int(100*(self.simNow-self.startTime)/(self.endTime-self.startTime))

    def Idle(self):
        dt = 1000
        for s in self.sources.values():
            ret = s.Idle(self.simNow)
            if ret is not None:
                dt = min(dt,ret)
        print 'Idle:',dt
        return dt
        
    def SysTime(self):
        now = datetime.datetime.utcnow()
        return calendar.timegm(now.utctimetuple()) + now.microsecond/1000000.0
