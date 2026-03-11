#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# Waypoint
# Send cow as waypoint

import gz4d
import sys
import os
import wx
import socket

class WaypointFrame(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self,parent,-1,title,size=(400,400))

        self.gz = gz4d.Connection()
        self.gz.Connect()
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        
        b = wx.Button(self,label='Get position',style=wx.BU_BOTTOM)
        sizer.Add(b)
        self.Bind(wx.EVT_BUTTON,self.OnGet,b)
        
        
        b = wx.Button(self,label='Send position',style=wx.BU_BOTTOM)
        sizer.Add(b)
        self.Bind(wx.EVT_BUTTON,self.OnSend,b)
        
        self.SetSizer(sizer)
        self.Layout()
        
        self.lat = None
        self.lon = None
        
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        
        
    def OnGet(self,event):
        print 'get'
        result = self.gz.SendCommand('gz posable get Window.main xy')
        print result
        if result[0]:
            parts = result[1].split()
            e = float(parts[0])
            n = float(parts[1])
            print e,n
            ll = self.gz.SendCommand(' '.join(('gz projection invert UTM.ZONE36',str(e),str(n))))
            if ll[0]:
                print ll[1]
                parts = ll[1].split()
                self.lon = float(parts[0])
                self.lat = float(parts[1])
                print self.lat, self.lon
   
    def OnSend(self,event):
        print 'send'
        if self.lat and self.lon:
            pass

class WaypointApp(wx.App):

    def OnInit(self):
        global f
        f = WaypointFrame(None,"Waypoint")
        f.Show()
        self.SetTopWindow(f)
        return True


if __name__ == "__main__":
    app = WaypointApp()
    app.MainLoop()

