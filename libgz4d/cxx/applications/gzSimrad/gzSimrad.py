#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import sys
import os

sys.path.append(os.path.join(sys.path[0],'../lib/python'))
import simrad
import gz4d

import wx
import socket
import datetime
import math
import struct

class GeoZui4D:
    def __init__(self):
        self.socket = None
        self.objects = {'nav':{}, 'targets':{}, 'surface':{}}

    def IsConnected(self):
        return self.socket is not None

    def Send(self,msg):
        if self.socket is not None:
            try:
                self.socket.send(msg)
                return self.socket.recv(2048).split('\n',1)
            except socket.error:
                self.socket = None

    def Connect(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect(('localhost',1974))
            ret = self.socket.recv(2048)
        except socket.error:
            self.socket = None

class MainFrame(simrad.MainFrame):
    def __init__(self, *args, **kwds):
        simrad.MainFrame.__init__(self, *args, **kwds)
        self.gz = GeoZui4D()

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnIdle)
        self.timer.Start(250)
        self.gzNavPathColourPickerCtrl = wx.ColourPickerCtrl(self.gzNavDetailPane,style=wx.CLRP_USE_TEXTCTRL)
        self.gzNavDetailPane.GetSizer().Add(self.gzNavPathColourPickerCtrl,1,wx.EXPAND,0)
        self.gzNavDetailPane.GetSizer().RecalcSizes()

    def OnIdle(self,e):
        if self.gz.IsConnected():
            ret = self.gz.Send('gztime get')
            if ret is not None:
                t = datetime.datetime.utcfromtimestamp(float(ret[1].strip()))
                self.gzStatusLabel.SetLabel('Connected!  GeoZui4D time: '+str(t)[:19])
            else:
                self.UpdateGZ()

    def OnFileConnect(self,e):
        fcDialog = ConnectDialog(self)
        if fcDialog.ShowModal() == wx.OK:
            self.SetCursor(wx.StockCursor(wx.CURSOR_WAIT))
            wx.SafeYield(self)
            servers = fcDialog.servers
            for s in servers:
                if s.GetType() == 'ME70':
                    page = ME70Panel(s,self.gz,self.mainNotebook)
                    self.mainNotebook.AddPage(page,'ME70')
                if s.GetType() == 'ER60':
                    page = EK60Panel(s,self.gz,self.mainNotebook)
                    self.mainNotebook.AddPage(page,'EK60')
            self.SetCursor(wx.NullCursor)

        fcDialog.Destroy()

    def OnFileExit(self,e):
        self.Close()

    def OnGZConnect(self,e):
        self.gz.Connect()
        self.UpdateGZ()

    def UpdateGZ(self):
        if self.gz.IsConnected():
            self.gzStatusLabel.SetLabel('Connected!')
        else:
            self.gzStatusLabel.SetLabel('(not connected)')

        for p in range(self.mainNotebook.GetPageCount()-1):
            self.mainNotebook.GetPage(p+1).UpdateGZ()

    def OnGzNavNew(self,e):
        if self.gz.IsConnected():
            label = self.newGZNavLabelTextCtrl.GetValue()
            ret = self.gz.Send('gz newobject BasicVessel ' + label)
            if ret is not None:
                oid = ret[1].strip()
                self.gz.objects['nav'][label] = {'oid':oid}
                self.gz.Send('gz tellObject '+oid+' projection Geographic')
                self.gz.Send('gz tellObject '+oid+' setRT')
                ret = self.gz.Send('gz tellObject ' + oid + ' connect')
                port = int(ret[1])
                s = gz4d.socket.UDPSocket()
                s.Connect(gz4d.socket.IPAddress('localhost',port))
                self.gz.objects['nav'][label]['socket'] = s
                model = self.newGZNavModelTextCtrl.GetValue()
                if model != '':
                    self.gz.Send('gz tellObject '+oid+' loadobject '+model)
                self.gzNavList.Append(label,self.gz.objects['nav'][label])
            self.UpdateGZ()

    def OnGzTargetsNew(self,e):
        if self.gz.IsConnected():
            label = self.newGZTargetsLabelTextCtrl.GetValue()
            ret = self.gz.Send('gz newobject FishSchool ' + label)
            if ret is not None:
                oid = ret[1].strip()
                self.gz.objects['targets'][label] = {'oid':oid}
                ret = self.gz.Send('gz tellObject ' + oid + ' connect')
                port = int(ret[1])
                s = gz4d.socket.UDPSocket()
                s.Connect(gz4d.socket.IPAddress('localhost',port))
                self.gz.objects['targets'][label]['socket'] = s
                print self.gz.objects['targets'][label]
                self.gzTargetsList.Append(label,self.gz.objects['targets'][label])
            self.UpdateGZ()


    def OnGzSurfaceNew(self,e):
        if self.gz.IsConnected():
            label = self.newGZSurfaceLabelTextCtrl.GetValue()
            ret = self.gz.Send('gz newobject XYZStream ' + label)
            if ret is not None:
                oid = ret[1].strip()
                self.gz.objects['surface'][label] = {'oid':oid}
                cellsize = self.newGZSurfaceCellSizeTextCtrl.GetValue()
                self.gz.Send('gz tellObject ' + oid + ' initialize ' + cellsize + ' swa ' + cellsize)
                ret = self.gz.Send('gz tellObject ' + oid + ' connect')
                port = int(ret[1])
                s = gz4d.socket.UDPSocket()
                s.Connect(gz4d.socket.IPAddress('localhost',port))
                self.gz.objects['surface'][label]['socket'] = s
                self.gzSurfaceList.Append(label,self.gz.objects['surface'][label])
            self.UpdateGZ()


class ConnectDialog(simrad.ConnectDialog):
    def __init__(self,*args, **kwds):
        simrad.ConnectDialog.__init__(self, *args, **kwds)
        self.sm = gz4d.sonar.simrad.ServerManager()
        al = gz4d.socket.IPAddress.List()
        for a in al:
            self.localAddressComboBox.Append(a)
        self.OnApplySettings(None)

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnIdle)
        self.timer.Start(1000)

    def OnIdle(self,e):
        servers = self.sm.GetList()
        serverStrings = []
        for s in servers:
            serverStrings.append(str(s))
            if self.serverListBox.FindString(str(s)) == wx.NOT_FOUND:
                self.serverListBox.Append(str(s),s)

        slist = self.serverListBox.GetItems()
        removeList = []
        for s in slist:
            if not s in serverStrings:
                removeList.append(s)

        for s in removeList:
            self.serverListBox.Delete(self.serverListBox.FindString(s))

    def OnConnect(self,e):
        serverIndecies = self.serverListBox.GetSelections()
        self.servers = []
        for s in serverIndecies:
            self.servers.append(self.serverListBox.GetClientData(s))
        self.EndModal(wx.OK)

    def OnSettingsChanged(self, event):
        try:
            port = int(self.portTextCtrl.GetValue())
            localAddress = self.localAddressComboBox.GetValue().encode('ascii')
            remoteAddress = self.remoteAddressTextCtrl.GetValue().encode('ascii')
            if port == self.port and localAddress == self.localAddress and remoteAddress == self.remoteAddress:
                self.applyButton.Disable()
            else:
                self.applyButton.Enable()
        except ValueError:
            self.applyButton.Disable()

    def OnApplySettings(self, event):
        try:
            self.port = int(self.portTextCtrl.GetValue())
            self.localAddress = self.localAddressComboBox.GetValue().encode('ascii')
            self.remoteAddress = self.remoteAddressTextCtrl.GetValue().encode('ascii')
            self.sm = gz4d.sonar.simrad.ServerManager(self.port,self.localAddress,self.remoteAddress)
            self.applyButton.Disable()
        except ValueError:
            pass

class ME70Panel(simrad.ME70Panel):
    def __init__(self, server, gz, *args, **kwds):
        simrad.ME70Panel.__init__(self, *args, **kwds)
        self.gz = gz
        self.UpdateGZ()
        self.client = server.CreateClient()
        self.client.Connect()
        self.channels = self.client.GetTransducer().GetChannels()
        self.channelList.Clear()
        for c in self.channels:
            c.subscriptions = {}
            self.channelList.Append(str(c),c)
        self.serverLabel.SetLabel(str(server))

        self.selectedChannel = None

        self.nav = self.client.GetPlatform().GetParameters()
        self.positionQueue = gz4d.utilities.TimeThreadedQueue()
        self.nav['position'].AddOutQueue(self.positionQueue)

        configs = self.client.GetBeamConfigurationManager().GetConfigurations()
        for c in configs:
            self.beamConfigurationList.Append(str(c),c)
        self.selectedBeamConfiguration = None

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnIdle)
        self.timer.Start(100)

    def UpdateGZ(self):
        if self.gz.IsConnected():
            self.navSendToGZCheckbox.Enable(True)

            navChoices = self.navDestinationComboBox.GetItems()
            for o in self.gz.objects['nav'].keys():
                if not o in navChoices:
                    self.navDestinationComboBox.Append(o,self.gz.objects['nav'][o])

            targetsChoices = self.bottomTargetsDestinationComboBox.GetItems()
            for o in self.gz.objects['targets'].keys():
                if not o in targetsChoices:
                    self.bottomTargetsDestinationComboBox.Append(o,self.gz.objects['targets'][o])
                    print self.gz.objects['targets'][o]

            surfaceChoices = self.bottomSurfaceDestinationComboBox.GetItems()
            for o in self.gz.objects['surface'].keys():
                if not o in surfaceChoices:
                    self.bottomSurfaceDestinationComboBox.Append(o,self.gz.objects['surface'][o])

        else:
            self.navSendToGZCheckbox.SetValue(False)
            self.navSendToGZCheckbox.Enable(False)

    def OnIdle(self,e):
        while not self.positionQueue.empty():
            t = self.positionQueue.pop()
            lat = self.nav['position']['lat'].Get(0.0,t)
            lon = self.nav['position']['lon'].Get(0.0,t)
            self.navTimeLabel.SetLabel(str(t)[:19])
            self.navPositionlabel.SetLabel(str(lat)+', '+str(lon))
            if self.navSendToGZCheckbox.GetValue():
                socket = self.navDestinationComboBox.GetClientData(self.navDestinationComboBox.GetSelection())['socket']
                if socket is not None:
                    packet = gz4d.data.Binary.Navigation_Position6DOF_Geographic()
                    packet.timestamp = t
                    packet.latitude = lat
                    packet.longitude = lon
                    packet.z = 0.0
                    packet.heading = self.nav['heading']['heading'].Get(0.0,t)
                    packet.pitch = 0.0
                    packet.roll = 0.0
                    socket.Send(packet)
        if self.selectedChannel is not None:
            params = self.selectedChannel.GetParameters()
            self.channelNameTextCtrl.SetValue(params['channel']['transducerName'].Get(''))
            self.channelFrequencyTextCtrl.SetValue(str(params['channel']['frequency'].Get(0.0)))
            self.channelDirectionXTextCtrl.SetValue(str(params['channel']['directionX'].Get(0.0)))
            self.channelDirectionYTextCtrl.SetValue(str(params['channel']['directionY'].Get(0.0)))
            self.channelBeamwidthXTextCtrl.SetValue(str(params['channel']['beamWidthX'].Get(0.0)))
            self.channelBeamwidthYTextCtrl.SetValue(str(params['channel']['beamWidthY'].Get(0.0)))
            self.channelGainTextCtrl.SetValue(str(params['channel']['gain'].Get(0.0)))
            self.channelPulseLengthTextCtrl.SetValue(str(params['channel']['pulseLength'].Get(0.0)))
            self.channelTransmitPowerTextCtrl.SetValue(str(params['channel']['transmitPower'].Get(0.0)))
            self.channelSoundSpeedTextCtrl.SetValue(str(params['channel']['soundSpeed'].Get(0.0)))

        if self.selectedBeamConfiguration is not None:
            params = self.selectedBeamConfiguration.GetParameters()
            self.selectedBeamConfigurationNameTextCtrl.SetValue(str(self.selectedBeamConfiguration))
            self.selectedBeamConfigurationDescriptionTextCtrl.SetValue(params['configuration']['description'].Get(''))
            self.selectedBeamConfigurationUpperFrequencyTextCtrl.SetValue(str(params['configuration']['frequencyUpper'].Get(0.0)))
            self.selectedBeamConfigurationLowerFrequencyTextCtrl.SetValue(str(params['configuration']['frequencyLower'].Get(0.0)))

        for i in range(self.channelList.GetCount()):
            c = self.channelList.GetClientData(i)
            try:
                b = c.subscriptions['bottom']
                while not b['queue'].empty():
                    targetSet = b['queue'].pop()
                    if self.sendBottomSurfaceCheckBox.GetValue():
                        socket = self.bottomSurfaceDestinationComboBox.GetClientData(self.bottomSurfaceDestinationComboBox.GetSelection())['socket']
                        if socket is not None:
                            bdy = c.GetParameters()['channel']['directionY'].Get(0.0)
                            bwy = c.GetParameters()['channel']['beamWidthY'].Get(0.0)
                            radius = abs(targetSet.targets[0].position[2]*(math.sin(math.radians(bdy-bwy/2.0))-math.sin(math.radians(bdy+bwy/2.0))))
    
                            packet = gz4d.data.Binary.Sonar_BottomDetection()
                            packet.timestamp = targetSet.time
                            packet.latitude = self.nav['position']['lat'].Get(0.0,targetSet.time)
                            packet.longitude = self.nav['position']['lon'].Get(0.0,targetSet.time)
                            packet.x = targetSet.targets[0].position[0]
                            packet.y = targetSet.targets[0].position[1]
                            packet.z = targetSet.targets[0].position[2]
                            packet.value = targetSet.targets[0].value
                            packet.radius = radius*1.5
                            socket.Send(packet)

                    if self.sendBottomPointsCheckBox.GetValue():
                        socket = self.bottomTargetsDestinationComboBox.GetClientData(self.bottomTargetsDestinationComboBox.GetSelection())['socket']

                        if socket is not None:
                            packet = gz4d.data.Binary.Target()
                            packet.timestamp = targetSet.time
                            packet.lat = self.nav['position']['lat'].Get(0.0,targetSet.time)
                            packet.lon = self.nav['position']['lon'].Get(0.0,targetSet.time)
                            packet.x = targetSet.targets[0].position[0]
                            packet.y = targetSet.targets[0].position[1]
                            packet.z = targetSet.targets[0].position[2]
                            packet.value = targetSet.targets[0].value
                            socket.Send(packet)

            except KeyError:
                pass

        activeBeamConfig = self.client.GetBeamConfigurationManager().GetParameters()['manager']['active']
        self.activeBeamConfigurationLabel.SetLabel(activeBeamConfig.Get('none'))

    def OnChannelSelected(self, e):
        selections = self.channelList.GetSelections()
        self.channelNameTextCtrl.SetValue('')
        self.channelFrequencyTextCtrl.SetValue('')
        self.channelDirectionXTextCtrl.SetValue('')
        self.channelDirectionYTextCtrl.SetValue('')
        self.channelBeamwidthXTextCtrl.SetValue('')
        self.channelBeamwidthYTextCtrl.SetValue('')
        self.channelGainTextCtrl.SetValue('')
        self.channelPulseLengthTextCtrl.SetValue('')
        self.channelTransmitPowerTextCtrl.SetValue('')
        self.channelSoundSpeedTextCtrl.SetValue('')

        if len(selections) == 1:
            self.selectedChannel = self.channelList.GetClientData(selections[0])
        else:
            self.selectedChannel = None

    def OnBeamConfigurationSelected(self, e):
        selections = self.beamConfigurationList.GetSelections()
        self.selectedBeamConfigurationNameTextCtrl.SetValue('')
        self.selectedBeamConfigurationDescriptionTextCtrl.SetValue('')
        self.selectedBeamConfigurationUpperFrequencyTextCtrl.SetValue('')
        self.selectedBeamConfigurationLowerFrequencyTextCtrl.SetValue('')

        if len(selections) == 1:
            self.selectedBeamConfiguration = self.beamConfigurationList.GetClientData(selections[0])
        else:
            self.selectedBeamConfiguration = None


    def OnClose(self, e):
        p = self.GetParent()
        p.DeletePage(p.GetSelection())

    def OnBottomSubscribe(self, e):
        if self.bottomSubscribeCheckBox.GetValue():
            for i in range(self.channelList.GetCount()):
                print self.channelList.GetString(i)
                c = self.channelList.GetClientData(i)
                if not c.subscriptions.has_key('bottom') or c.subscriptions['bottom'] is None:
                    c.subscriptions['bottom'] = {}
                    c.subscriptions['bottom']['queue'] = gz4d.sonar.simrad.TargetSetThreadedQueue()
                    c.subscriptions['bottom']['subscription'] = c.GetBottomDetection()
                    c.subscriptions['bottom']['subscription'].AddOutQueue(c.subscriptions['bottom']['queue'])


class EK60Panel(simrad.EK60Panel):
    def __init__(self, server, gz, *args, **kwds):
        simrad.EK60Panel.__init__(self, *args, **kwds)
        self.gz = gz
        self.client = server.CreateClient()
        self.client.Connect()
        self.channels = self.client.GetTransducer().GetChannels()
        self.channelList.Clear()
        for c in self.channels:
            self.channelList.Append(str(c),c)
        self.serverLabel.SetLabel(str(server))

    def OnClose(self, e):
        p = self.GetParent()
        p.DeletePage(p.GetSelection())

app = wx.PySimpleApp(0)
wx.InitAllImageHandlers()
mainFrame = MainFrame(None, -1, "")
app.SetTopWindow(mainFrame)
mainFrame.Show()
app.MainLoop()


