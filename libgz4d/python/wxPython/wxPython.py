#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import gz4d
import wx
import wx.glcanvas

class Canvas(wx.glcanvas.GLCanvas):
    def __init__(self, parent):
        wx.glcanvas.GLCanvas.__init__(self, parent,-1,attribList=[wx.glcanvas.WX_GL_RGBA, wx.glcanvas.WX_GL_DOUBLEBUFFER])
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.Bind(wx.EVT_SIZE,self.OnSize)
        self.Bind(wx.EVT_MOTION,self.OnMotion)
        self.Bind(wx.EVT_LEFT_UP,self.OnLeftUp)
        self.Bind(wx.EVT_LEFT_DOWN,self.OnLeftDown)
        self.Bind(wx.EVT_MIDDLE_UP,self.OnMiddleUp)
        self.Bind(wx.EVT_MIDDLE_DOWN,self.OnMiddleDown)
        self.Bind(wx.EVT_RIGHT_UP,self.OnRightUp)
        self.Bind(wx.EVT_RIGHT_DOWN,self.OnRightDown)
        self.Bind(wx.EVT_MOUSEWHEEL,self.OnWheel)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)

        self.canvas = gz4d.scene.Canvas()

    def OnPaint(self,event):
        dc = wx.PaintDC(self)
        self.SetCurrent()
        self.OnDraw()
        return

    def OnDraw(self):
        self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.Draw))
        self.SwapBuffers()

    def OnSize(self,event):
        self.SetCurrent()
        w,h = self.GetClientSizeTuple()
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.Size,0,w,h)))

    def OnMotion(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.Mouse,0,event.GetX(),event.GetY())))

    def OnLeftUp(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,0,event.GetX(),event.GetY(),False)))

    def OnLeftDown(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,0,event.GetX(),event.GetY(),True)))
        event.Skip()

    def OnMiddleUp(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,1,event.GetX(),event.GetY(),False)))

    def OnMiddleDown(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,1,event.GetX(),event.GetY(),True)))

    def OnWheel(self,event):
        if event.GetWheelRotation() < 0:
            up = False
        else:
            up = True
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseWheel,0,event.GetX(),event.GetY(),up)))

    def OnIdle(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.Idle)))

    def OnRightUp(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,2,event.GetX(),event.GetY(),False)))

    def OnRightDown(self,event):
        self.HandleEvents(self.canvas.HandleEvent(gz4d.Event(gz4d.EventType.MouseButton,2,event.GetX(),event.GetY(),True)))

    def OnEraseBackground(self, event): 
        pass # To avoid flashing on Windows... we're double-buffering anyway. 

    def HandleEvents(self, events):
        for e in events:
            print e
            if e.type == gz4d.EventType.Draw:
                self.Refresh()
