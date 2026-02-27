#!/usr/bin/env python

import sys
import os
import wx

sys.path.append(os.path.join(sys.path[0],'../lib/python'))
import geozui4d

class GZMainFrame(geozui4d.MainFrame):
    def __init__(self, *args, **kwds):
        geozui4d.MainFrame.__init__(self, *args, **kwds)


if __name__ == "__main__":
    app = wx.PySimpleApp(0)
    wx.InitAllImageHandlers()
    mainFrame = GZMainFrame(None, -1, "")
    app.SetTopWindow(mainFrame)
    mainFrame.Show()
    app.MainLoop()
