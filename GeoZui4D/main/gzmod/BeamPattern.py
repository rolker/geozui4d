#!/usr/bin/env python
#Boa:App:BoaApp
# gz
# BeamPattern
# Interface to simulated array of acoustic elements.

import wx

import BeamPatternFrame

modules ={u'BeamPatternFrame': [1, 'Main frame of Application', u'BeamPatternFrame.py']}

class BoaApp(wx.App):
    def OnInit(self):
        self.main = BeamPatternFrame.create(None)
        self.main.Show()
        self.SetTopWindow(self.main)
        return True

def main():
    application = BoaApp(0)
    application.MainLoop()

if __name__ == '__main__':
    main()
