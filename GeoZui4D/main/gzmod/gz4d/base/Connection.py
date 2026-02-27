import socket
import sys
from GzError import GzError
import os


class Connection:
    class AddressFormatError(GzError):
        pass

    class MissingAddressError(GzError):
        pass

    def __init__(self):
        self.ip = None
        self.port = None
        self.appdir = None
        self.datadir = None
        self.gzs = None
        self.address = None

    def parseAddress(self,address):
        aparts = address.split(':')
        if len(aparts) == 2:
            self.ip = aparts[0]
            self.port = int(aparts[1])
            self.address = address
        else:
            raise Connection.AddressFormatError('Expected address of the form address:port')

    def Connect(self, address = None):

        if address:
            self.parseAddress(address)

        if not self.ip:
            if len(sys.argv) >= 2:
                self.parseAddress(sys.argv[1])

        if not (self.ip and self.port):
            raise Connection.MissingAddressError()

        self.gzs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.gzs.connect((self.ip,self.port))
        self.gzs.recv(2048)
        self.appdir = self.SendCommand('global appdir; set appdir')[1]
        self.datadir = self.SendCommand('global datadir; set datadir')[1]
        if self.datadir == '':
            self.datadir = os.getcwd()


    def SendCommand(self,cmd):
        self.gzs.send(str(cmd))
        res = self.gzs.recv(16384)
        result = [False,'']
        if res:
            if res.splitlines()[0] == '0':
                result[0] = True
            result[1] = res.split("\n",1)[1].strip()
            return result
        return None

    def GetTime(self):
        result = self.SendCommand("gztime get")
        if result[0]:
            return float(result[1])
        return 0.0

    def GetTimeScale(self):
        result = self.SendCommand("gztime timescale")
        if result[0]:
            return float(result[1])
        return 1.0

    def GetPosition(self):
        result = self.SendCommand("gz posable get Window.main xyz")
        if result[0]:
            return [float(result[1].split()[0]),float(result[1].split()[1]),float(result[1].split()[2])]
        return [0.0,0.0,0.0]

    def GetAttitude(self):
        result = self.SendCommand("gz posable get Window.main hpr")
        if result[0]:
            return [float(result[1].split()[0]),float(result[1].split()[1]),float(result[1].split()[2])]
        return [0.0,0.0,0.0]

    def GetScale(self):
        result = self.SendCommand("gz posable get Window.main S")
        if result[0]:
            return float(result[1])
        return 1.0

    def Log(self, msg, scope='py', level=2):
        cmd = ''.join(('gz log "',scope,'" ',str(level),' "',msg,'"'))
        self.SendCommand(cmd)
