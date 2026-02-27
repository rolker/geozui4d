#!/usr/bin/env/ python

import sys
import os

import x3dManagerUI
import wx

import gz4d
import vrml

class X3DDetailPanel(x3dManagerUI.X3DDetailPanelBase):
    def __init__(self, *args, **kwds):
        x3dManagerUI.X3DDetailPanelBase.__init__(self, *args, **kwds)
        self.gzc = None
        self.gzId = None

    def log(self,message):
        self.x3dLogText.write(message+'\n');

    def OpenFile(self,filename):
        self.log('Opening '+filename+'...')
        self.filenameStaticText.SetLabel(filename)
        f = open(filename)
        self.scene = vrml.Scene()
        self.scene.parse(f)
        #self.log(self.scene.PrettyString())
        self.SendToGZ()

    def sendCmd(self,cmd):

        if self.gzId is None:
            self.gzId = self.gzc.SendCommand('gz newobject X3D scene')[1]
            self.log('gzId: '+self.gzId)

        return self.gzc.SendCommand(' '.join(('gz tellObject',self.gzId,cmd)))

    def SendToGZ(self,node = None, parent = None):
        if node == None:
            node = self.scene.root
            self.scene.root.gzid = 'root'
        else:
            self.newNode(node,parent)

        for n,a in node.attributes.iteritems():
            t = vrml.vrml2.nodes[node.nodeType][n][0]
            if t.startswith('SF'):
                if t == 'SFNode':
                    self.SendToGZ(a,node)
                else:
                    c = vrml.vrml2.types[t[2:]]['count']
                    if c > 1:
                        #print a
                        val = ''
                        for v in a:
                            val = ' '.join((val,str(v)))
                        val = val.join(('"','"'))
                        self.sendCmd(' '.join(('setValue',node.gzid,n,val)))
                    else:
                        self.sendCmd(' '.join(('setValue',node.gzid,n,str(a))))
            else:
                if t == 'MFNode':
                    for c in a:
                        self.SendToGZ(c,node)
                else:
                    for i in range(len(a)):
                        c = vrml.vrml2.types[t[2:]]['count']
                        if c > 1:
                            #print a
                            val = ''
                            for v in a[i]:
                                val = ' '.join((val,str(v)))
                            val = val.join(('"','"'))
                            self.sendCmd(' '.join(('setValue',node.gzid,n,str(i),val)))
                        else:
                            self.sendCmd(' '.join(('setValue',node.gzid,n,str(i),str(a[i]))))

    def newNode(self,n,parent):
        if parent is None:
            pid = 'root'
        else:
            pid = parent.gzid

        ret = self.sendCmd(' '.join(('newNode',pid,n.nodeType)))
        n.gzid = ret[1]
        self.log(' '.join(('Node',n.gzid,'created')))


class X3DManagerFrame(x3dManagerUI.X3DManagerFrameBase):
    def __init__(self, *args, **kwds):
        x3dManagerUI.X3DManagerFrameBase.__init__(self, *args, **kwds)

         # if we were launched from GZ, connect so we can later get datadir value.
        self.gzc = gz4d.Connection()
        try:
            self.gzc.Connect()
        except gz4d.GzError:
            self.gzc = None

    def redirectStdStreams(self):
        sys.stdout = self.mainLogText
        sys.stderr = self.mainLogText

    def printStatus(self):
        if self.gzc is None:
            print 'Not connected'
        else:
            print 'Connected'

    def OpenFile(self,filename):
        p = X3DDetailPanel(self.topNotebook)
        l = os.path.basename(filename)
        self.topNotebook.AddPage(p,l)
        p.gzc = self.gzc
        p.OpenFile(filename)

    def OnOpen(self,evt):
        fdlg = wx.FileDialog(self,style=wx.OPEN)
        if self.gzc is not None:
            fdlg.SetDirectory(self.gzc.datadir)
        ret = fdlg.ShowModal()
        if ret == wx.ID_OK:
            self.OpenFile(filename=fdlg.GetPath())

class x3dManagerApp(wx.App):

    def OnInit(self):
        self.mainFrame = X3DManagerFrame(None)
        self.mainFrame.Show()
        self.SetTopWindow(self.mainFrame)
        self.mainFrame.redirectStdStreams()
        self.mainFrame.printStatus()
        return True
