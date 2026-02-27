#!/usr/bin/env python
#Boa:App:BoaApp

import wx

import sys
sys.path.append('../')
import simMbFishFrame

modules ={u'simMbFishFrame': [1, 'Main frame of Application', u'simMbFishFrame.py']}

class BoaApp(wx.App):
    def OnInit(self):
        self.main = simMbFishFrame.create(None)
        self.main.Show()
        self.SetTopWindow(self.main)
        return True

def main():
    application = BoaApp(0)
    application.MainLoop()

if __name__ == '__main__':
    main()
