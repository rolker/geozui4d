# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

import sys
import os

sys.path.append(os.path.join(sys.path[0],'../lib/python2.5/site-packages'))
sys.path.append(os.path.join(sys.path[0],'../lib/python2.4/site-packages'))

import gz4d
import wx
import wx.glcanvas

class X3dCanvas(wx.glcanvas.GLCanvas):
    def __init__(self, parent):
        wx.glcanvas.GLCanvas.__init__(self, parent,-1)
        self.Bind(wx.EVT_PAINT,self.OnPaint)
        self.Bind(wx.EVT_SIZE,self.OnSize)
        self.Bind(wx.EVT_MOTION,self.OnMotion)
        self.Bind(wx.EVT_LEFT_UP,self.OnLeftUp)
        self.Bind(wx.EVT_LEFT_DOWN,self.OnLeftDown)
        self.Bind(wx.EVT_MOUSEWHEEL,self.OnWheel)
        self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)

        self.sceneManager = gz4d.SceneManager()

        self.widgets = gz4d.Widgets()
        self.screen = gz4d.Screen()
        self.screen.setWidgets(self.widgets)

        self.startX = None

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnTimer)
        self.timer.Start(100)

    def loadX3D(self,x3dFile):
        self.sceneManager.addX3D().Load(gz4d.Filename(x3dFile))


    def OnPaint(self,event):
        dc = wx.PaintDC(self)
        self.SetCurrent()
        self.OnDraw()
        return

    def OnDraw(self):
        self.sceneManager.drawScreen(self.screen)
        self.SwapBuffers()

    def OnSize(self,event):
        self.SetCurrent()
        w,h = self.GetClientSizeTuple()
        self.screen.SetViewport(gz4d.Viewport(w,h))

    def NormalizeMouse(self,x,y):
        w,h = self.GetClientSizeTuple()
        return (-1.0+((x-w)*2.0)/float(w),-1.0+((y-h)*2.0)/float(h))

    def OnMotion(self,event):
        if self.startX is not None:
            x,y = self.NormalizeMouse(event.GetX(),event.GetY())
            self.widgets.moveRotationAt(x,y)
            self.Refresh()

    def OnLeftUp(self,event):
        self.startX = None
        x,y = self.NormalizeMouse(event.GetX(),event.GetY())
        self.widgets.endRotationAt(x,y)

    def OnLeftDown(self,event):
        self.startX = event.GetX()
        x,y = self.NormalizeMouse(event.GetX(),event.GetY())
        self.widgets.startRotationAt(x,y)

        event.Skip()

    def OnWheel(self,event):
        if event.GetWheelRotation() < 0:
            self.screen.getTransform().Scale(.5)
        else:
            self.screen.getTransform().Scale(1.5)

    def OnTimer(self,event):
        self.widgets.OnIdle()
        self.Refresh()

    def OnEraseBackground(self, event): 
        pass # To avoid flashing on Windows... we're double-buffering anyway. 
