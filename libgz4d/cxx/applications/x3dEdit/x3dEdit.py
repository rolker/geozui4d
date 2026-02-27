#!/usr/bin/env python
#Boa:App:BoaApp

import sys
import wx

import x3dEditFrame

modules ={u'x3dEditFrame': [1, 'Main frame of Application', u'x3dEditFrame.py']}

class BoaApp(wx.App):
    def OnInit(self):
        self.main = x3dEditFrame.create(None)
        self.main.Show()
        self.SetTopWindow(self.main)
        for f in sys.argv[1:]:
            self.main.canvas.loadX3D(f)
        return True

def main():
    application = BoaApp(0)
    application.MainLoop()

if __name__ == '__main__':
    main()
