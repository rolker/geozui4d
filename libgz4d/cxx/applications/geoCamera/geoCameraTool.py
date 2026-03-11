#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-

import xml.etree.ElementTree
import os
import gz4d
import geoCameraTool_ui
import wx
import wx.glcanvas
from OpenGL.GL import *
from OpenGL.GLUT import *
import math
import sys

class Project:
    def __init__(self,fname,treeCtrl,statusbar):
        self.typeName = 'Project'

        self.project_file = fname 
        self.project_path = os.path.dirname(self.project_file)
        self.label = os.path.basename(fname)

        treeCtrlRoot = treeCtrl.AddRoot(self.label,data=wx.TreeItemData(self))

        tree = xml.etree.ElementTree.parse(self.project_file)
        root = tree.getroot()
        nav = root.findall('Navigation')
        if len(nav):
            nav = nav[0]
            statusbar.SetStatusText("loading nav...")
            wx.SafeYield()
            self.track = gz4d.nav.Track(os.path.join(self.project_path,nav.attrib['file']))
            statusbar.SetStatusText("loading nav... Done!")
            wx.SafeYield()

        conf = root.findall('Configuration')
        if len(conf):
            conf = conf[0]
            self.config = {}
            cam = conf.findall('Camera')
            if len(cam):
                cam = cam[0]
                self.config['Camera'] = (float(cam.attrib['x']),float(cam.attrib['y']),float(cam.attrib['z']))
            wl = conf.findall('Waterline')
            if len(wl):
                wl = wl[0]
                self.config['Waterline'] = float(wl.attrib['z'])
            self.config['WaterlineTweak'] = 0.0

        mos = root.findall('Mosaic')
        if len(mos):
            mos = mos[0]
            self.mosaic = {}
            self.mosaic['path'] = mos.attrib['path']
            self.mosaic['tile_size'] = float(mos.attrib['tile_size'])
            self.mosaic['base_resolution'] = float(mos.attrib['base_resolution'])

        ortho = root.findall('Ortho')
        if len(ortho):
            ortho = ortho[0]
            self.ortho = {}
            self.ortho['path'] = ortho.attrib['path']
            self.ortho['resolution'] = float(ortho.attrib['resolution'])
            self.ortho['max_distance'] = float(ortho.attrib['max_distance'])


        isets = root.findall('ImageSet')

        for s in isets:
            ImageSet(s,self.project_path,self.track,treeCtrl,treeCtrlRoot,statusbar)

        statusbar.SetStatusText('done loading.')
        wx.SafeYield()


class ImageSet:
    def __init__(self, node, path, track, treeCtrl, treeCtrlParent, statusbar):
        self.typeName = 'ImageSet'

        self.label = node.attrib['label']

        statusbar.SetStatusText('loading image set '+self.label+'...')
        wx.SafeYield()

        treeCtrlItem = treeCtrl.AppendItem(treeCtrlParent,self.label,data=wx.TreeItemData(self))

        t = gz4d.utilities.Time(self.label)
        self.nav_fix = track(t)

        self.path = node.attrib['path']
        if self.path[0] != '/':
            self.path = os.path.join(path,self.path)

        for i in node.findall('Image'):
            Image(i, self.path, self.nav_fix, track, treeCtrl, treeCtrlItem, statusbar)

class Image:
    def __init__(self, node, path, base_nav, track, treeCtrl, treeCtrlParent, statusbar):
        self.typeName = 'Image'

        self.fname = node.attrib['file']
        self.label = os.path.basename(self.fname)


        self.treeCtrlItem = treeCtrl.AppendItem(treeCtrlParent,self.label,data=wx.TreeItemData(self))

        self.path = path
        ptu_path = os.path.join(self.path,self.fname).rsplit('_',1)[0]+'_ptu.txt'
        for l in file(ptu_path):
            if l.startswith('pan'):
                self.pan = float(l.split()[1])
            if l.startswith('tilt'):
                self.tilt = float(l.split()[1])
            if l.startswith('2'):
                self.time_stamp = l.strip()
                t = gz4d.utilities.Time(self.time_stamp)
                self.nav_fix = track(t)

        self.xTweak = 0.0
        self.yTweak = 0.0
        self.panTweak = 0.0
        self.tiltTweak = 0.0
        self.headingTweak = 0.0
        self.pitchTweak = 0.0
        self.rollTweak = 0.0

        self.texture = None
        self.load = False
        self.width = 0.0
        self.height = 0.0
        self.imageFrame = None
        self.active = False

    def LoadTexture(self):
        if self.texture is None and self.load:
            image = gz4d.image.ImageRGBA8()
            image.Load(os.path.join(self.path, self.fname))
            self.texture = gz4d.glo.Texture()
            self.texture.Bind()
            r = image.GetRaster()
            if r.GetColumns() > r.GetRows():
                self.width = 1.0
                self.height = r.GetRows()/float(r.GetColumns())
            else:
                self.height = 1.0
                self.width = r.GetColumns()/float(r.GetRows())

            self.texture.Image(r)

            self.texture.MinFilter(GL_LINEAR)
            self.texture.MagFilter(GL_NEAREST)
            self.texture.WrapS(GL_CLAMP_TO_BORDER)
            self.texture.WrapT(GL_CLAMP_TO_BORDER)
            self.texture.Parameterfv(GL_TEXTURE_BORDER_COLOR,(0.0,0.0,0.0,0.0))
        if not self.load and self.texture is not None:
            self.texture = None

        return self.texture

    def hto3(self,h):
        return gz4d.Pointd(h[0]/h[3],h[1]/h[3],h[2]/h[3])

    def GenerateProjection(self,d,config):
        exif = gz4d.exif.Exif(os.path.join(self.path, self.fname))
        
        f = exif.GetFloat("exif","FocalLength")/1000.0
        
        x = exif.GetShort("exif","PixelXDimension")
        y = exif.GetShort("exif","PixelYDimension")
        
        xr = exif.GetFloat("exif","FocalPlaneXResolution")/ 0.0254
        yr = exif.GetFloat("exif","FocalPlaneYResolution")/ 0.0254
        
        w = x/xr
        h = y/yr
        
        #f *= 10.0
        #w *= 10.0
        #h *= 10.0

        projection = gz4d.Matrix4d()
        projection[0,0] = 2.0*f/w
        projection[0,2] = 0.0
        projection[1,1] = 2.0*f/h
        projection[1,2] = 0.0
        projection[2,2] = -(d+f)/(d-f)
        projection[2,3] = -(2.0*d*f)/(d-f)
        projection[3,2] = -1.0

        inv_projection = gz4d.Matrix4d()
        inv_projection[0,0] = w/(2.0*f)
        inv_projection[1,1] = h/(2.0*f)
        inv_projection[2,3] = -1
        inv_projection[3,2] = -(d-f)/(2.0*d*f)
        inv_projection[3,3] = (d+f)/(2.0*d*f)

        viewport_shift = gz4d.Translationd(0.5,0.5,0.0)
        viewport_scale = gz4d.Scaled(0.5,0.5,1.0)

        coord_transform = gz4d.Rotationd(math.radians(-90.0),gz4d.Pointd(1.0,0.0,0.0))

        pan = gz4d.Rotationd(-math.radians(self.pan+self.panTweak),gz4d.Pointd(0.0,0.0,1.0))
        tilt = gz4d.Rotationd(-math.radians((self.tilt+self.tiltTweak)),gz4d.Pointd(1.0,0.0,0.0))

        camera_position = gz4d.Translationd(-config['Camera'][0],-config['Camera'][1],-config['Camera'][2])
        cog = gz4d.Translationd(0.0,0.0,config['Waterline']+config['WaterlineTweak'])

        heading = gz4d.Rotationd(math.radians((self.nav_fix.heading+self.headingTweak)),gz4d.Pointd(0.0,0.0,1.0))

        # pitch positive nose up, roll positive right down
        pitch = gz4d.Rotationd(-math.radians(self.nav_fix.pitch+self.pitchTweak),gz4d.Pointd(1.0,0.0,0.0))
        roll = gz4d.Rotationd(-math.radians(self.nav_fix.roll+self.rollTweak),gz4d.Pointd(0.0,1.0,0.0))

        self.transformation = viewport_shift.GetMatrix()*viewport_scale.GetMatrix()*projection*coord_transform.GetMatrix()*tilt.GetMatrix()*pan.GetMatrix()*camera_position.GetMatrix()*roll.GetMatrix()*pitch.GetMatrix()*heading.GetMatrix()*cog.GetMatrix()
        self.inv_transformation = cog.GetInverseMatrix()*heading.GetInverseMatrix()*pitch.GetInverseMatrix()*roll.GetInverseMatrix()*camera_position.GetInverseMatrix()*pan.GetInverseMatrix()*tilt.GetInverseMatrix()*coord_transform.GetInverseMatrix()*inv_projection*viewport_scale.GetInverseMatrix()*viewport_shift.GetInverseMatrix()

        cam_heading = math.radians(self.nav_fix.heading+self.headingTweak - (self.pan+self.panTweak))
        cos_ch = math.cos(cam_heading)
        sin_ch = math.sin(cam_heading)
        self.orthoQuad = ((-d*cos_ch,d*sin_ch),(d*cos_ch,-d*sin_ch),(d*cos_ch+d*sin_ch,d*cos_ch-d*sin_ch),(-d*cos_ch+d*sin_ch,d*cos_ch+d*sin_ch))

class ControlPoint:
    def __init__(self):
        self.typeName = 'ControlPoint'

        self.label = "control point"
        self.images = {}
        self.active = False

    def AddImage(self,img,c):
        self.images[img.label] = {'enabled':True,'x':0.5,'y':0.5,'treeCtrlItem':c}

    def Enable(self,img):
        self.images[img.label]['enabled'] = True

    def Disable(self,img):
        self.images[img.label]['enabled'] = False

class ImageCanvas(wx.glcanvas.GLCanvas):
    def __init__(self, parent, context = None):
        canvas = wx.glcanvas.GLCanvasWithContext(parent,context,-1,attribList=[wx.glcanvas.WX_GL_RGBA, wx.glcanvas.WX_GL_DOUBLEBUFFER])
        self.PostCreate(canvas)

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

        self.image = None

        self.zoom = 1.0
        self.panning = False
        self.lastx = None
        self.lasty = None
        self.panx = 0.0
        self.pany = 0.0


    def SetImage(self, image):
        self.image = image
        self.Refresh()

    def OnPaint(self,event):
        dc = wx.PaintDC(self)
        self.SetCurrent()
        self.OnDraw()

    def OnDraw(self):
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(self.l, self.r, self.b, self.t, -1, 1)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

        glViewport(0, 0, self.w, self.h)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        glScalef(self.zoom,self.zoom,self.zoom)
        glTranslatef(self.panx,self.pany,0.0)

        if self.image is not None:
            texture = self.image.LoadTexture()
            if texture is not None:
                texture.Bind()

                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
                glEnable(GL_TEXTURE_2D)
            else:
                glDisable(GL_TEXTURE_2D)
            glColor3f(1.0,1.0,1.0)

    def OnSize(self,event):
        self.SetCurrent()
        self.w,self.h = self.GetClientSizeTuple()

        glViewport(0, 0, self.w, self.h)
        self.l = -1.0
        self.r = 1.0
        self.t = 1.0
        self.b = -1.0
        if self.w>self.h:
            self.ppu = self.h/2.0
            self.l *= self.w/float(self.h)
            self.r *= self.w/float(self.h)
        else:
            self.ppu = self.w/2.0
            self.b *= self.h/float(self.w)
            self.t *= self.h/float(self.w)

        if self.panx > self.r:
            self.panx = self.r
        if self.panx < self.l:
            self.panx = self.l
        if self.pany > self.t:
            self.pany = self.t
        if self.pany < self.b:
            self.pany = self.b

        self.Refresh()


    def OnMotion(self,event):
        if self.panning:
            x = event.GetX()
            y = event.GetY()
            dx = x - self.lastx
            dy = -(y - self.lasty)
            newx = self.panx + (2.0/self.zoom)*self.r*dx/float(self.w)
            newy = self.pany + (2.0/self.zoom)*self.t*dy/float(self.h)

            if  newx <= self.r and newx >= self.l:
                self.panx = newx
                self.lastx = x

            if newy <= self.t and newy >= self.b:
                self.pany += (2.0/self.zoom)*self.t*dy/float(self.h)
                self.lasty = y

            self.Refresh()

    def OnLeftUp(self,event):
        self.panning = False

    def OnLeftDown(self,event):
        self.panning = True
        self.lastx = event.GetX()
        self.lasty = event.GetY()


    def OnMiddleUp(self,event):
        pass

    def OnMiddleDown(self,event):
        pass

    def OnWheel(self,event):
        if event.GetWheelRotation() < 0:
            up = False
        else:
            up = True

        if up:
            self.zoom *= 1.1
        else:
            self.zoom /= 1.1
        if self.zoom < 1.0:
            self.zoom = 1.0
        if self.zoom > 25.0:
            self.zoom = 25.0
        self.Refresh()

    def OnRightUp(self,event):
        pass

    def OnRightDown(self,event):
        pass

    def DrawCursor(self,x,y,size):
        delta = size/(self.ppu*self.zoom)
        glLineWidth(2.0)
        glBegin(GL_LINE_LOOP)
        glVertex2f(x-delta,y)
        glVertex2f(x,y-delta)
        glVertex2f(x+delta,y)
        glVertex2f(x,y+delta)
        glEnd()


class PerspectiveImageCanvas(ImageCanvas):
    def __init__(self, parent, context=None):
        ImageCanvas.__init__(self,parent,context)

    def OnRightDown(self,event):
        #print event.GetX(),event.GetY()
        x = (self.r*2.0*(-0.5+event.GetX()/float(self.w))/self.zoom)-self.panx
        y = (self.t*2.0*(0.5-event.GetY()/float(self.h))/self.zoom)-self.pany
        #print x,y


        if self.image is not None and self.image.load:
            if self.r/self.t > self.image.width/self.image.height:
                h = self.t
                w = self.t*self.image.width/self.image.height
            else:
                w = self.r
                h = self.r*self.image.height/self.image.width

            for cp in self.control_points:
                if cp.active and cp.images.has_key(self.image.label):
                    if x <= w and x >= -w and y <= h and y >= -h:
                        cp.images[self.image.label]['x'] = (w+x)/(2.0*w)
                        cp.images[self.image.label]['y'] = (h+y)/(2.0*h)
                        cp.images[self.image.label]['enabled'] = True
                    else:
                        cp.images[self.image.label]['enabled'] = False
                    wx.GetTopLevelParent(self).UpdateSelected()

    def OnDraw(self):
        ImageCanvas.OnDraw(self)
        if self.image is not None and self.image.load:
            glDisable(GL_BLEND)
            if self.r/self.t > self.image.width/self.image.height:
                h = self.t
                w = self.t*self.image.width/self.image.height
            else:
                w = self.r
                h = self.r*self.image.height/self.image.width
            glBegin(GL_QUADS)

            glTexCoord2f(0.0,1.0)
            glVertex2f(-w,h)
            glTexCoord2f(0.0,0.0)
            glVertex2f(-w,-h)
            glTexCoord2f(1.0,0.0)
            glVertex2f(w,-h)
            glTexCoord2f(1.0,1.0)
            glVertex2f(w,h)
            glEnd()

            glDisable(GL_TEXTURE_2D)

            for cp in self.control_points:
                if cp.images.has_key(self.image.label) and cp.images[self.image.label]['enabled']:
                    if cp.active:
                        glColor3f(0.0,1.0,0.0)
                    else:
                        glColor3f(1.0,1.0,1.0)
                    x = (-.5+cp.images[self.image.label]['x'])*2.0*w
                    y = (-.5+cp.images[self.image.label]['y'])*2.0*h
                    self.DrawCursor(x,y,5.0)

        self.SwapBuffers()


class OrthoImageCanvas(ImageCanvas):
    def __init__(self, parent, context=None):
        ImageCanvas.__init__(self,parent, context)

    def OnRightDown(self,event):
        #print event.GetX(),event.GetY()
        x = (self.r*2.0*(-0.5+event.GetX()/float(self.w))/self.zoom)-self.panx
        y = (self.t*2.0*(0.5-event.GetY()/float(self.h))/self.zoom)-self.pany
        #print x,y

        if self.image is not None and self.image.load:
            p = gz4d.Vector4d()
            p[0] = x*1000.0
            p[1] = y*1000.0
            p[2] = 0.0
            p[3] = 1.0
            pi = self.image.hto3(self.image.transformation*p)
            #print pi
            x = pi[0]
            y = pi[1]

            for cp in self.control_points:
                if cp.active and cp.images.has_key(self.image.label):
                    if x <= 1.0 and x >= 0.0 and y <= 1.0 and y >= 0.0:
                        cp.images[self.image.label]['x'] = x
                        cp.images[self.image.label]['y'] = y
                        cp.images[self.image.label]['enabled'] = True
                    else:
                        cp.images[self.image.label]['enabled'] = False
                    wx.GetTopLevelParent(self).UpdateSelected()


    def OnDraw(self):
        ImageCanvas.OnDraw(self)
        if self.image is not None and self.image.load:
            glDisable(GL_BLEND)
            glMatrixMode(GL_TEXTURE)
            gz4d.glo.LoadMatrix(self.image.transformation)

            glBegin(GL_QUADS)
            #for c in ((-1000.0,1000.0),(-1000.0,-1000.0),(1000.0,-1000.0),(1000.0,1000.0)):
            for c in self.image.orthoQuad:
                glTexCoord2f(c[0],c[1])
                glVertex2f(c[0]/1000.0,c[1]/1000.0)
            glEnd()

            glLoadIdentity()
            glMatrixMode(GL_MODELVIEW)

            glDisable(GL_TEXTURE_2D)
            for cp in self.control_points:
                if cp.images.has_key(self.image.label) and cp.images[self.image.label]['enabled']:
                    if cp.active:
                        glColor3f(0.0,1.0,0.0)
                    else:
                        glColor3f(1.0,1.0,1.0)
                    x = cp.images[self.image.label]['x']
                    y = cp.images[self.image.label]['y']
                    p = gz4d.Vector4d()
                    p[0] = x
                    p[1] = y
                    p[2] = 0.0
                    p[3] = 1.0
                    
                    p_near = self.image.hto3(self.image.inv_transformation*p)
                    p[2] = 1.0
                    p_far = self.image.hto3(self.image.inv_transformation*p)

                    if p_far[2] < p_near[2]:
                        r = -p_near[2]/(p_far[2]-p_near[2])
                        x = p_near[0]+r*(p_far[0]-p_near[0])
                        y = p_near[1]+r*(p_far[1]-p_near[1])
                        self.DrawCursor(x/1000.0,y/1000.0,5.0)

        self.SwapBuffers()

class CombinedOrthoImageCanvas(ImageCanvas):
    def __init__(self, parent, context=None):
        ImageCanvas.__init__(self,parent, context)
        self.images = []

    def AddImage(self, image):
        if not image in self.images:
            self.images.append(image)
            self.Refresh()

    def RemoveImage(self, image):
        if image in self.images:
            self.images.remove(image)
            self.Refresh()

    def OnDraw(self):
        ImageCanvas.OnDraw(self)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_COLOR,GL_DST_COLOR)
        glBlendEquation(GL_MAX)

        lat = 0.0
        lon = 0.0
        count = 0

        for i in self.images:
            if i is not None:
                lat += i.nav_fix.latitude
                lon += i.nav_fix.longitude
                count += 1

        if count:
            lat /= float(count)
            lon /= float(count)

            ref = gz4d.geo.LocalENU(gz4d.geo.LatLonPoint(lat,lon,0.0))
            

            for i in self.images:
                if i is not None:
                    offset = ref(gz4d.geo.ECEFPoint(gz4d.geo.LatLonPoint(i.nav_fix.latitude,i.nav_fix.longitude,0.0)))
                    texture = i.LoadTexture()
                    texture.Bind()

                    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

                    glColor3f(1.0,1.0,1.0)
                    glEnable(GL_TEXTURE_2D)

                    glMatrixMode(GL_TEXTURE)
                    gz4d.glo.LoadMatrix(i.transformation)

                    glBegin(GL_QUADS)
                    #for c in ((-1000.0,1000.0),(-1000.0,-1000.0),(1000.0,-1000.0),(1000.0,1000.0)):
                    for c in i.orthoQuad:
                        glTexCoord2f(c[0],c[1])
                        glVertex2f((offset[0]+c[0])/1000.0,(offset[1]+c[1])/1000.0)
                    glEnd()

                    glLoadIdentity()
                    glMatrixMode(GL_MODELVIEW)
                    
                    glDisable(GL_TEXTURE_2D)
                    for cp in self.control_points:
                        if cp.images.has_key(i.label) and cp.images[i.label]['enabled']:
                            if cp.active:
                                if i.active:
                                    glColor3f(0.0,1.0,0.0)
                                else:
                                    glColor3f(0.0,0.0,1.0)
                            else:
                                glColor3f(1.0,1.0,1.0)
                            x = cp.images[i.label]['x']
                            y = cp.images[i.label]['y']
                            p = gz4d.Vector4d()
                            p[0] = x
                            p[1] = y
                            p[2] = 0.0
                            p[3] = 1.0
                            
                            p_near = i.hto3(i.inv_transformation*p)
                            p[2] = 1.0
                            p_far = i.hto3(i.inv_transformation*p)
                            
                            if p_far[2] < p_near[2]:
                                r = -p_near[2]/(p_far[2]-p_near[2])
                                x = p_near[0]+r*(p_far[0]-p_near[0])
                                y = p_near[1]+r*(p_far[1]-p_near[1])
                                self.DrawCursor((offset[0]+x)/1000.0,(offset[1]+y)/1000.0,5.0)

                    glEnable(GL_TEXTURE_2D)

        self.SwapBuffers()


class MainFrame(geoCameraTool_ui.geoCameraToolMainWindow ):
    def __init__(self, *args, **kwds):
        geoCameraTool_ui.geoCameraToolMainWindow.__init__(self, *args, **kwds)

        self.controlPointsTreeCtrl.AddRoot("control points")
        self.control_points = []

        self.mosaicImageCanvas = CombinedOrthoImageCanvas(self.mosaicPanel)
        self.mosaicImageCanvas.control_points = self.control_points

        self.perspectiveImageCanvas = None
        self.baseImageCanvas = None

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self.mosaicImageCanvas, 1, wx.EXPAND, 0)
        self.mosaicPanel.SetSizer(sizer)

        glutInit()

        self.Layout()

        self.project_file = None
        self.project_path = None
        self.selected_image = None
        self.selected_cp = None
        self.selected_cp_item = None
        self.UpdateSelected()


    def Open(self,fn):
        Project(fn,self.projectTreeCtrl,self.mainFrame_statusbar)

    def OnFileOpen(self,e):
        d = wx.FileDialog(self,wildcard='*.xml',style=wx.OPEN|wx.HIDE_READONLY)
        ret = d.ShowModal()
        if ret == wx.ID_OK:
            self.Open(str(d.GetPath()))

    def OnTreeSelectionChanged(self,e):
        item = self.projectTreeCtrl.GetPyData(e.GetItem())
        if self.selected_image is not None:
            self.selected_image.active = False
            
        if item.typeName == 'Image':
            self.selected_image = item
            self.selected_image.active = True
            if self.selected_cp_item is not None:
                self.controlPointsTreeCtrl.SelectItem(self.selected_cp_item)
        else:
            self.selected_image = None

        self.UpdateSelected()

    def OnTreeItemActivated(self,e):
        item = self.projectTreeCtrl.GetPyData(e.GetItem())
        if item.typeName == 'Image':
            if self.baseImageCanvas is None:
                context = self.mosaicImageCanvas.GetContext()
                self.baseImageCanvas = PerspectiveImageCanvas(self.basePanel,context)
                self.baseImageCanvas.control_points = self.control_points

                bsizer = wx.BoxSizer(wx.HORIZONTAL)
                bsizer.Add(self.baseImageCanvas, 1, wx.EXPAND, 0)
                self.baseImagePanel.SetSizer(bsizer)
                self.Layout()
            self.baseImageCanvas.SetImage(item)


    def OnNewControlPoint(self,e):
        cp = ControlPoint()
        self.controlPointsTreeCtrl.AppendItem(self.controlPointsTreeCtrl.GetRootItem(),cp.label,data=wx.TreeItemData(cp))
        self.control_points.append(cp)


    def OnCPTreeSelectionChanged(self,e):
        self.selected_cp_item = e.GetItem()
        if self.selected_cp is not None:
            self.selected_cp.active = False

        self.selected_cp = self.controlPointsTreeCtrl.GetPyData(self.selected_cp_item)
        if self.selected_cp.typeName is 'Image':
            img = self.selected_cp
            self.selected_cp_item = self.controlPointsTreeCtrl.GetItemParent(e.GetItem())
            self.selected_cp = self.controlPointsTreeCtrl.GetPyData(self.selected_cp_item)
            self.projectTreeCtrl.SelectItem(img.treeCtrlItem)

        self.selected_cp.active = True

        self.UpdateSelected()

    def UpdateSelected(self):
        if self.selected_image is not None:
            self.cpImageSizer_staticbox.Enable()
            if self.perspectiveImageCanvas is None:
                context = self.mosaicImageCanvas.GetContext()
                
                self.perspectiveImageCanvas = PerspectiveImageCanvas(self.perspectivePanel,context)
                self.perspectiveImageCanvas.control_points = self.control_points
                self.orthoImageCanvas = OrthoImageCanvas(self.orthoPanel,context)
                self.orthoImageCanvas.control_points = self.control_points
                
                psizer = wx.BoxSizer(wx.HORIZONTAL)
                psizer.Add(self.perspectiveImageCanvas, 1, wx.EXPAND, 0)
                self.perspectivePanel.SetSizer(psizer)
                
                
                osizer = wx.BoxSizer(wx.HORIZONTAL)
                osizer.Add(self.orthoImageCanvas, 1, wx.EXPAND, 0)
                self.orthoPanel.SetSizer(osizer)
                self.Layout()
                
            self.mosaicCheckBox.SetValue(self.selected_image in self.mosaicImageCanvas.images)
            self.selected_image.load = True
            
            self.timeTextCtrl.SetValue(self.selected_image.time_stamp)
            self.latTextCtrl.SetValue(str(self.selected_image.nav_fix.latitude))
            self.lonTextCtrl.SetValue(str(self.selected_image.nav_fix.longitude))
            self.waterLineTextCtrl.SetValue(str(self.projectTreeCtrl.GetPyData(self.projectTreeCtrl.GetRootItem()).config['Waterline']))
            self.headingTextCtrl.SetValue(str(self.selected_image.nav_fix.heading))
            self.pitchTextCtrl.SetValue(str(self.selected_image.nav_fix.pitch))
            self.rollTextCtrl.SetValue(str(self.selected_image.nav_fix.roll))
            self.panTextCtrl.SetValue(str(self.selected_image.pan))
            self.tiltTextCtrl.SetValue(str(self.selected_image.tilt))
            
            self.xTweakSlider.SetValue(self.selected_image.xTweak*10)
            self.yTweakSlider.SetValue(self.selected_image.yTweak*10)
            project = self.projectTreeCtrl.GetPyData(self.projectTreeCtrl.GetRootItem())
            self.waterLineTweakSlider.SetValue(project.config['WaterlineTweak']*100)
            self.headingTweakSlider.SetValue(self.selected_image.headingTweak*100)
            self.pitchTweakSlider.SetValue(self.selected_image.pitchTweak*100)
            self.rollTweakSlider.SetValue(self.selected_image.rollTweak*100)
            self.panTweakSlider.SetValue(self.selected_image.panTweak*100)
            self.tiltTweakSlider.SetValue(self.selected_image.tiltTweak*100)
            
            self.UpdateTweakTextCtrls()
            self.UpdateProjection()
                
        else:
            self.cpImageSizer_staticbox.Disable()

        if self.perspectiveImageCanvas is not None:
            self.perspectiveImageCanvas.SetImage(self.selected_image)
            self.orthoImageCanvas.SetImage(self.selected_image)

        if self.selected_cp is not None:
            self.cpDetailsSizer_staticbox.Enable()
            self.cpLabelTextCtrl.SetValue(self.selected_cp.label)
            self.cpLabelTextCtrl.Enable()
            self.cpLabelLabel.Enable()
        else:
            self.cpDetailsSizer_staticbox.Disable()
            self.cpLabelTextCtrl.Clear()
            self.cpLabelTextCtrl.Disable()
            self.cpLabelLabel.Disable()

        if self.selected_cp is not None and self.selected_image is not None:
            self.cpImageEnabledCheckBox.Enable()
        else:
            self.cpImageEnabledCheckBox.Disable()

        if self.selected_cp is not None and self.selected_image is not None and self.selected_cp.images.has_key(self.selected_image.label):
            self.cpImageXTextCtrl.SetValue(str(self.selected_cp.images[self.selected_image.label]['x']))
            self.cpImageYTextCtrl.SetValue(str(self.selected_cp.images[self.selected_image.label]['y']))
        else:
            self.cpImageXTextCtrl.Clear()
            self.cpImageYTextCtrl.Clear()

        cp_selected_image_enabled = self.selected_cp is not None and self.selected_image is not None and self.selected_cp.images.has_key(self.selected_image.label) and self.selected_cp.images[self.selected_image.label]['enabled']

        self.cpImageEnabledCheckBox.SetValue(cp_selected_image_enabled)
        self.cpImageXLabel.Enable(cp_selected_image_enabled)
        self.cpImageXTextCtrl.Enable(cp_selected_image_enabled)
        self.cpImageYLabel.Enable(cp_selected_image_enabled)
        self.cpImageYTextCtrl.Enable(cp_selected_image_enabled)


    def OnCPImageEnabledChanged(self,e):
        if self.cpImageEnabledCheckBox.GetValue():
            if not self.selected_cp.images.has_key(self.selected_image.label):
                self.selected_cp.AddImage(self.selected_image, self.controlPointsTreeCtrl.AppendItem(self.selected_cp_item,self.selected_image.label,data=wx.TreeItemData(self.selected_image)))
            self.selected_cp.Enable(self.selected_image)
            self.controlPointsTreeCtrl.SetItemBold(self.selected_cp.images[self.selected_image.label]['treeCtrlItem'],True)
        else:
            self.selected_cp.Disable(self.selected_image)
            self.controlPointsTreeCtrl.SetItemBold(self.selected_cp.images[self.selected_image.label]['treeCtrlItem'],False)

        self.UpdateSelected()


    def UpdateTweakTextCtrls(self):
        self.xTweakTextCtrl.SetValue(str(self.selected_image.xTweak))
        self.yTweakTextCtrl.SetValue(str(self.selected_image.yTweak))
        project = self.projectTreeCtrl.GetPyData(self.projectTreeCtrl.GetRootItem())
        self.waterLineTweakTextCtrl.SetValue(str(project.config['WaterlineTweak']))
        self.panTweakTextCtrl.SetValue(str(self.selected_image.panTweak))
        self.tiltTweakTextCtrl.SetValue(str(self.selected_image.tiltTweak))
        self.headingTweakTextCtrl.SetValue(str(self.selected_image.headingTweak))
        self.pitchTweakTextCtrl.SetValue(str(self.selected_image.pitchTweak))
        self.rollTweakTextCtrl.SetValue(str(self.selected_image.rollTweak))

    def OnTweak(self,e):
        self.selected_image.xTweak = self.xTweakSlider.GetValue()/10.0
        self.selected_image.yTweak = self.yTweakSlider.GetValue()/10.0
        project = self.projectTreeCtrl.GetPyData(self.projectTreeCtrl.GetRootItem())
        project.config['WaterlineTweak'] = self.waterLineTweakSlider.GetValue()/100.0
        self.selected_image.panTweak = self.panTweakSlider.GetValue()/100.0
        self.selected_image.tiltTweak = self.tiltTweakSlider.GetValue()/100.0
        self.selected_image.headingTweak = self.headingTweakSlider.GetValue()/100.0
        self.selected_image.pitchTweak = self.pitchTweakSlider.GetValue()/100.0
        self.selected_image.rollTweak = self.rollTweakSlider.GetValue()/100.0

        self.UpdateTweakTextCtrls()
        self.UpdateProjection()

    def UpdateProjection(self):
        project = self.projectTreeCtrl.GetPyData(self.projectTreeCtrl.GetRootItem())
        o = project.ortho
        c = project.config
        self.selected_image.GenerateProjection(o['max_distance'],c)
        for i in self.mosaicImageCanvas.images:
            if i is not None:
                i.GenerateProjection(o['max_distance'],c)
        self.Refresh()

    def OnClearTweaks(self,e):
        self.xTweakSlider.SetValue(0)
        self.yTweakSlider.SetValue(0)
        self.waterLineTweakSlider.SetValue(0)
        self.headingTweakSlider.SetValue(0)
        self.pitchTweakSlider.SetValue(0)
        self.rollTweakSlider.SetValue(0)
        self.panTweakSlider.SetValue(0)
        self.tiltTweakSlider.SetValue(0)
        self.OnTweak(None)

    def OnMosaicInclusion(self,e):
        if self.mosaicCheckBox.IsChecked():
            self.mosaicImageCanvas.AddImage(self.selected_image)
            
        else:
            self.mosaicImageCanvas.RemoveImage(self.selected_image)
        self.projectTreeCtrl.SetItemBold(self.selected_image.treeCtrlItem,self.mosaicCheckBox.IsChecked())

if __name__ == '__main__':

    app = wx.PySimpleApp(0)
    wx.InitAllImageHandlers()
    mainFrame = MainFrame(None, -1, "")
    app.SetTopWindow(mainFrame)
    mainFrame.Show()
    if len(sys.argv) > 1:
        mainFrame.Open(sys.argv[1])
    app.MainLoop()

