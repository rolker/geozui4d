#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import sys
import os

sys.path.append(os.path.join(sys.path[0],'../lib/python'))

import gz4d
import gzwx
import wx

class myGLCanvas(gzwx.Canvas):
    def __init__(self, parent):
        gzwx.Canvas.__init__(self, parent)

        self.vm = gz4d.scene.ViewManager()
        self.canvas.SetContent(self.vm)

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnIdle)
        self.timer.Start(25)

    def OnRightDown(self,event):
        d = wx.FileDialog(self,wildcard='*.x3d',style=wx.OPEN|wx.HIDE_READONLY)
        ret = d.ShowModal()
        if ret == wx.ID_OK:
            x3d = gz4d.x3d.X3D()
            x3d.Load(str(d.GetPath()))

            si = gz4d.scene.SceneInstance()
            si.SetScene(x3d)
            v = gz4d.scene.View()
            v.SetSceneInstance(si)
            self.vm.AddView(v)

            s = self.window.NewScreen()
            s.SetDrawable(x3d)

    def OnRightUp(self,event):
        pass


def main():
    app = wx.PySimpleApp()
    frame = wx.Frame(None,-1,'x3dView',wx.DefaultPosition,wx.Size(800,600))
    canvas = myGLCanvas(frame)
    frame.Show()
    app.MainLoop()

if __name__ == '__main__': main()
