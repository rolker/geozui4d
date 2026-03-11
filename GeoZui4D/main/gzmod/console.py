#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import wx
import sys

import os
import stat
import platform

import gz4d

def getScripts(appdir):
    # scan the scripts subdir for suitable scripts to add to our launch panel
    # a suitable script is identified by a line containing '# gz' appearing before
    # any line not starting with #. If the next line after '# gz' also starts with '#'
    # it is treated as a label. The next line after the label is treated as a description
    # if it also begins with #

    # I need to think of a way to have "installed system modules" with user scripts added at runtime...
    
    sdir = os.path.join(appdir,"gzmod")
    pscripts = os.listdir(sdir)
    pscripts.sort()

    ret = []
    
    for ps in pscripts:
        pspath = os.path.join(sdir,ps)
        isd = os.stat(pspath)[stat.ST_MODE]
        if not stat.S_ISDIR(isd) and ps.endswith('.py'):
            tmp = open(pspath)
            l = tmp.readline().strip()
            while l and l[0] == '#':
                if l.strip() == "# gz":
                    l = tmp.readline().strip().split(' ',1)
                    if l and l[0] == "#":
                        sname = l[1]
                    else:
                        sname = ps.strip()
                    l = tmp.readline().strip().split(' ',1)
                    if l and l[0] == "#":
                        sdesc = l[1]
                    else:
                        sdesc = "(no description)"
                    ret.append({'name': sname, 'description': sdesc, 'path': pspath})
                    break
                l = tmp.readline()
    return ret


class gzConsoleFrame(wx.Frame):
    def __init__(self, parent, title):
        wx.Frame.__init__(self,parent,-1,title,size=(640,480))
        
        self.gz = gz4d.Connection()
        self.gz.Connect()
        
        self.history = []
        self.historyCursor = 0
        self.currentCommand = ""
        
        # notebook with tab for console, and one to launch scripts
        notebook = wx.Notebook(self)
        consolePanel = wx.Panel(notebook)
        scriptsPanel = wx.Panel(notebook)
        notebook.AddPage(consolePanel, "GZConsole")
        notebook.AddPage(scriptsPanel, "Scripts")
        
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(notebook, 1, wx.EXPAND|wx.ALL, 5)
        
        # console widgets
        self.resultsText = wx.TextCtrl(consolePanel,style = wx.TE_MULTILINE)
        self.commandText = wx.TextCtrl(consolePanel,style = wx.TE_PROCESS_ENTER)
        
        self.commandText.Bind(wx.EVT_CHAR,self.OnCommandEdit,self.commandText)
        
        consoleSizer = wx.BoxSizer(wx.VERTICAL)
        consolePanel.SetSizer(consoleSizer)
        
        consoleSizer.Add(self.resultsText, 1, wx.ALL | wx.EXPAND, 2)
        consoleSizer.Add(self.commandText, 0, wx.ALL | wx.EXPAND, 2)
        
        self.commandText.SetFocus()
        
        self.Bind(wx.EVT_TEXT_ENTER, self.OnCommand,self.commandText)
        
        self.scripts = getScripts(self.gz.appdir)
        
        
        if len(self.scripts) > 0:
            scriptsizer = wx.FlexGridSizer(len(self.scripts),2)
            scriptsizer.AddGrowableCol(1,1)
            scriptsPanel.SetSizer(scriptsizer)
            
            for s in self.scripts:
                b = wx.Button(scriptsPanel,label=s['name'],style=wx.BU_BOTTOM,name=s['path'])
                scriptsizer.Add(item=b,flag=wx.EXPAND|wx.ALL,border=2)
                l = wx.StaticText(scriptsPanel,label=s['description'])
                bs = wx.BoxSizer(wx.VERTICAL)
                bs.Add((1,1),proportion=1,flag=wx.EXPAND)
                bs.Add(item=l,flag=wx.ALL,border=5)
                bs.Add((1,1),proportion=1,flag=wx.EXPAND)
                scriptsizer.Add(item=bs,flag=wx.EXPAND)
                self.Bind(wx.EVT_BUTTON,self.OnLoadScript,b)
                        
        self.SetSizer(sizer)
        self.Layout()
        
    def OnCommand(self,evt):
        if self.commandText.GetValue() == "":
            return
        
        self.history.append(str(self.commandText.GetValue()))
        
        result = self.gz.SendCommand(str(self.commandText.GetValue()))
        self.resultsText.SetDefaultStyle(wx.TextAttr(wx.BLACK))
        self.resultsText.AppendText(self.commandText.GetValue() + "\n")
        if result[0]:
            self.resultsText.SetDefaultStyle(wx.TextAttr(wx.BLUE))
        else:
            self.resultsText.SetDefaultStyle(wx.TextAttr(wx.RED))
        self.resultsText.AppendText(result[1] + "\n")
        self.commandText.SetValue("")
        self.historyCursor = len(self.history)
        self.currentCommand = ""

    def OnCommandEdit(self,evt):
        k = evt.GetKeyCode()

        if k == 317:
            if self.historyCursor == len(self.history):
                self.currentCommand = self.commandText.GetValue()
            if self.historyCursor > 0:
                self.historyCursor -= 1
                self.commandText.SetValue(self.history[self.historyCursor])
                self.commandText.SetInsertionPointEnd()

        if k == 319:
            if self.historyCursor < len(self.history):
                self.historyCursor += 1
                if self.historyCursor < len(self.history):
                    self.commandText.SetValue(self.history[self.historyCursor])
                else:
                    self.commandText.SetValue(self.currentCommand)
                self.commandText.SetInsertionPointEnd()
        evt.Skip()

    def OnLoadScript(self,evt):
        s = evt.GetEventObject().GetName()
        print 'Launching',os.path.basename(s),'...'
        if platform.uname()[0] in ('Windows', 'Microsoft'):
            os.system('cd')
            p = 'python.exe '
        else:
            p = ''
        os.system(p+'"' + s + '" ' + self.gz.address + ' &')

class gzConsole(wx.App):

    def OnInit(self):
        f = gzConsoleFrame(None,"GZConsole")
        f.Show()
        self.SetTopWindow(f)
        return True

if __name__ == '__main__':
    app = gzConsole()
    app.MainLoop()
