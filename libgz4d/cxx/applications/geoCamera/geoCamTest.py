#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-

import geoCamTest_ui
import wx
import wx.glcanvas
from OpenGL.GL import *
from OpenGL.GLUT import *
import gz4d
import math

class TestGLCanvas(wx.glcanvas.GLCanvas):
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
        self.gl_init = False

        self.pan = 0.0
        self.tilt = 0.0
        self.heading = 0.0
        self.pitch = 0.0
        self.roll = 0.0

        exif = gz4d.exif.Exif('/home/roland/work/mishapX/geoCamera/2010-06-01_18.58.32.215_canon.jpg')

        #f = exif.GetFloat("exif","FocalLength")/1000.0
        f = exif.GetFloat("exif","FocalLength")/10.0

        x = exif.GetShort("exif","PixelXDimension")
        y = exif.GetShort("exif","PixelYDimension")

        #xr = exif.GetFloat("exif","FocalPlaneXResolution")/ 0.0254
        #yr = exif.GetFloat("exif","FocalPlaneYResolution")/ 0.0254
        xr = exif.GetFloat("exif","FocalPlaneXResolution")/ 0.0254
        yr = exif.GetFloat("exif","FocalPlaneYResolution")/ 0.0254

        w = 100.0*x/xr
        h = 100.0*y/yr

        d = 100000.0


        self.cfh = []
        self.cfh.append(gz4d.Vector4d())
        self.cfh[-1][0] = -(10.0*w/2.0)
        self.cfh[-1][1] = (10.0*h/2.0)
        self.cfh[-1][2] = -(10.0*f)
        self.cfh[-1][3] = 1.0

        self.cfh.append(gz4d.Vector4d())
        self.cfh[-1][0] = (10.0*w/2.0)
        self.cfh[-1][1] = (10.0*h/2.0)
        self.cfh[-1][2] = -(10.0*f)
        self.cfh[-1][3] = 1.0

        self.cfh.append(gz4d.Vector4d())
        self.cfh[-1][0] = -(10.0*w/2.0)
        self.cfh[-1][1] = -(10.0*h/2.0)
        self.cfh[-1][2] = -(10.0*f)
        self.cfh[-1][3] = 1.0

        self.cfh.append(gz4d.Vector4d())
        self.cfh[-1][0] = (10.0*w/2.0)
        self.cfh[-1][1] = -(10.0*h/2.0)
        self.cfh[-1][2] = -(10.0*f)
        self.cfh[-1][3] = 1.0

        self.projection = gz4d.Matrix4d()
        self.projection[0,0] = 2.0*f/w
        self.projection[0,2] = 0.0
        self.projection[1,1] = 2.0*f/h
        self.projection[1,2] = 0.0
        self.projection[2,2] = -(d+f)/(d-f)
        self.projection[2,3] = -(2.0*d*f)/(d-f)
        self.projection[3,2] = -1.0

        self.camera_position = gz4d.Translationd(1.03,-22.08,-25.16)
        self.cog = gz4d.Translationd(0.0,0.0,-2.84)

        self.viewport_shift = gz4d.Translationd(0.5,0.5,0.0)
        self.viewport_scale = gz4d.Scaled(0.5,0.5,1.0)

        self.coord_transform = gz4d.Rotationd(math.radians(-90.0),gz4d.Pointd(1.0,0.0,0.0))


    def hto3(self,h):
        return gz4d.Pointd(h[0]/h[3],h[1]/h[3],h[2]/h[3])

    def OnPaint(self,event):
        dc = wx.PaintDC(self)
        self.SetCurrent()
        self.OnDraw()

    def OnDraw(self):
        if not self.gl_init:
            glutInit()
            self.gl_init = True

        print '\t'.join(('pan:'+str(self.pan),'tilt:'+str(self.tilt),'h:'+str(self.heading),'p:'+str(self.pitch),'r:'+str(self.roll)))

        pan = gz4d.Rotationd(-math.radians(self.pan),gz4d.Pointd(0.0,0.0,1.0))
        tilt = gz4d.Rotationd(-math.radians(self.tilt),gz4d.Pointd(1.0,0.0,0.0))

        heading = gz4d.Rotationd(math.radians(self.heading),gz4d.Pointd(0.0,0.0,1.0))

        # pitch positive nose up, roll positive right down
        pitch = gz4d.Rotationd(-math.radians(self.pitch),gz4d.Pointd(1.0,0.0,0.0))
        roll = gz4d.Rotationd(-math.radians(self.roll),gz4d.Pointd(0.0,1.0,0.0))

        transformation = self.viewport_shift.GetMatrix()*self.viewport_scale.GetMatrix()*self.projection*self.coord_transform.GetMatrix()*tilt.GetMatrix()*pan.GetMatrix()*self.camera_position.GetMatrix()*roll.GetMatrix()*pitch.GetMatrix()*heading.GetMatrix()*self.cog.GetMatrix()

        rev_transformation = self.cog.GetInverseMatrix()*heading.GetInverseMatrix()*pitch.GetInverseMatrix()*roll.GetInverseMatrix()*self.camera_position.GetInverseMatrix()*pan.GetInverseMatrix()*tilt.GetInverseMatrix()*self.coord_transform.GetInverseMatrix()

        dph = gz4d.Vector4d()
        dph[0] = 0.0
        dph[1] = 0.0
        dph[2] = 0.0
        dph[3] = 1.0

        dpmh = gz4d.Vector4d()
        dpmh[0] = 0.0
        dpmh[1] = -22.08
        dpmh[2] = 0.0
        dpmh[3] = 1.0


        dph = heading.GetInverseMatrix()*pitch.GetInverseMatrix()*roll.GetInverseMatrix()*self.camera_position.GetInverseMatrix()*dph
        dpmh = heading.GetInverseMatrix()*pitch.GetInverseMatrix()*roll.GetInverseMatrix()*self.camera_position.GetInverseMatrix()*dpmh
        dp = self.hto3(dph)
        dpm = self.hto3(dpmh)
        print dp

        cf = []

        for c in self.cfh:
            cf.append(self.hto3(rev_transformation*c))

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glColor3f(1.0,0.0,0.0)
        glPointSize(5.0)
        glBegin(GL_POINTS)

        v = gz4d.Vector4d()
        v[2] = 0.0
        v[3] = 1.0

        for i in range(100):
            x = -1000.0 + 20*i
            v[0] = x
            for j in range(100):
                y = -1000.0 + 20*j
                v[1] = y
                cv = transformation*v
                if cv[3] > 0:
                    cp = self.hto3(cv)
                    if cp[0] > 0.0 and cp[0] < 1.0 and cp[1] > 0.0 and cp[1] < 1.0 and cp[2] < 1.0 and cp[2] > 0.0:
                        glColor3f(1.0,cp[0],cp[1])
                        glVertex2f(x/1000.0,y/1000.0)

        glColor3f(1.0,1.0,1.0)
        glVertex2f(0.0,0.0)
        glColor3f(0.0,1.0,0.0)
        glVertex2f(dp[0]/1000.0,dp[1]/1000.0)
        glEnd()

        glColor3f(1.0,1.0,1.0)
        glBegin(GL_LINE_LOOP)
        glVertex2f(-1.6,1.0)
        glVertex2f(-1.0,1.0)
        glVertex2f(-1.0,0.4)
        glVertex2f(-1.6,0.4)
        glEnd()

        glBegin(GL_LINE_LOOP)
        glVertex2f(-1.6,0.3)
        glVertex2f(-1.0,0.3)
        glVertex2f(-1.0,-0.3)
        glVertex2f(-1.6,-0.3)
        glEnd()

        glBegin(GL_LINE_LOOP)
        glVertex2f(-1.6,-0.4)
        glVertex2f(-1.0,-0.4)
        glVertex2f(-1.0,-1.0)
        glVertex2f(-1.6,-1.0)
        glEnd()

        glBegin(GL_POINTS)
        glVertex2f(-1.3,0.7)
        glVertex2f(-1.3,-0.2)
        glVertex2f(-1.3,-0.9)

        glColor3f(0.8,0.8,0.8)
        glVertex2f(-1.3+dpm[0]/100.0,0.7+dpm[1]/100.0)
        glVertex2f(-1.3+dpm[1]/100.0,-0.2+dpm[2]/100.0)
        glVertex2f(-1.3+dpm[0]/100.0,-0.9+dpm[2]/100.0)


        glColor3f(0.0,1.0,0.0)
        glVertex2f(-1.3+dp[0]/100.0,0.7+dp[1]/100.0)
        glVertex2f(-1.3+dp[1]/100.0,-0.2+dp[2]/100.0)
        glVertex2f(-1.3+dp[0]/100.0,-0.9+dp[2]/100.0)
        glEnd()

        glColor3f(0.5,0.5,0.5)
        glBegin(GL_LINES)
        for c in cf:
            glVertex2f(-1.3+dp[0]/100.0,0.7+dp[1]/100.0)
            glVertex2f(-1.3+c[0]/100.0,0.7+c[1]/100.0)

            glVertex2f(-1.3+dp[1]/100.0,-0.2+dp[2]/100.0)
            glVertex2f(-1.3+c[1]/100.0,-0.2+c[2]/100.0)

            glVertex2f(-1.3+dp[0]/100.0,-0.9+dp[2]/100.0)
            glVertex2f(-1.3+c[0]/100.0,-0.9+c[2]/100.0)
        glEnd()

        self.SwapBuffers()

    def OnSize(self,event):
        self.SetCurrent()
        w,h = self.GetClientSizeTuple()

        glViewport(0, 0, w, h)
        l = -1.0
        r = 1.0
        t = 1.0
        b = -1.0
        if w>h:
            l *= w/float(h)
            r *= w/float(h)
        else:
            b *= h/float(w)
            t *= h/float(w)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(l, r, b, t, -1, 1)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        self.Refresh()


    def OnMotion(self,event):
        pass

    def OnLeftUp(self,event):
        pass

    def OnLeftDown(self,event):
        pass

    def OnMiddleUp(self,event):
        pass

    def OnMiddleDown(self,event):
        pass

    def OnWheel(self,event):
        if event.GetWheelRotation() < 0:
            up = False
        else:
            up = True

    def OnRightUp(self,event):
        pass

    def OnRightDown(self,event):
        pass


class MainFrame(geoCamTest_ui.GeoCamTestMainFrame ):
    def __init__(self, *args, **kwds):
        geoCamTest_ui.GeoCamTestMainFrame.__init__(self, *args, **kwds)

        self.gl = TestGLCanvas(self)
        self.GetSizer().Add(self.gl, 1, wx.EXPAND, 0)
        self.Layout()

    def OnControlsChanged(self, e):
        self.gl.pan = self.panSlider.GetValue()/10.0
        self.panTextCtrl.SetValue(str(self.gl.pan))
        self.gl.tilt = self.tiltSlider.GetValue()/10.0
        self.tiltTextCtrl.SetValue(str(self.gl.tilt))
        self.gl.heading = self.headingSlider.GetValue()/10.0
        self.headingTextCtrl.SetValue(str(self.gl.heading))
        self.gl.pitch = self.pitchSlider.GetValue()/10.0
        self.pitchTextCtrl.SetValue(str(self.gl.pitch))
        self.gl.roll = self.rollSlider.GetValue()/10.0
        self.rollTextCtrl.SetValue(str(self.gl.roll))

        self.gl.Refresh()

if __name__ == '__main__':
    app = wx.PySimpleApp(0)
    wx.InitAllImageHandlers()
    mainFrame = MainFrame(None, -1, "")
    app.SetTopWindow(mainFrame)
    mainFrame.Show()
    app.MainLoop()

