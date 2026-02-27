#Boa:Frame:X3dEditFrame

import x3dCanvas

import wx

def create(parent):
    return X3dEditFrame(parent)

[wxID_X3DEDITFRAME, wxID_X3DEDITFRAMEDETAILSSCROLLEDWINDOW, 
 wxID_X3DEDITFRAMEDETAILSTREECTRL, wxID_X3DEDITFRAMEGLPANEL, 
 wxID_X3DEDITFRAMEMAINSPLITTERWINDOW, 
] = [wx.NewId() for _init_ctrls in range(5)]

class X3dEditFrame(wx.Frame):

    def _init_coll_mainSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.mainSplitterWindow, 1, border=0, flag=wx.EXPAND)

    def _init_coll_detailsSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.detailsTreeCtrl, 1, border=0, flag=wx.EXPAND)

    def _init_sizers(self):
        # generated method, don't edit
        self.mainSizer = wx.BoxSizer(orient=wx.HORIZONTAL)

        self.glSizer = wx.BoxSizer(orient=wx.VERTICAL)

        self.detailsSizer = wx.BoxSizer(orient=wx.VERTICAL)

        self._init_coll_mainSizer_Items(self.mainSizer)
        self._init_coll_detailsSizer_Items(self.detailsSizer)

        self.SetSizer(self.mainSizer)
        self.detailsScrolledWindow.SetSizer(self.detailsSizer)
        self.glPanel.SetSizer(self.glSizer)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_X3DEDITFRAME, name=u'X3dEditFrame',
              parent=prnt, pos=wx.Point(585, 357), size=wx.Size(930, 525),
              style=wx.DEFAULT_FRAME_STYLE, title=u'X3D Edit')
        self.SetClientSize(wx.Size(930, 525))
        self.SetAutoLayout(True)

        self.mainSplitterWindow = wx.SplitterWindow(id=wxID_X3DEDITFRAMEMAINSPLITTERWINDOW,
              name=u'mainSplitterWindow', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(930, 525), style=wx.SP_3D)
        self.mainSplitterWindow.SetAutoLayout(True)

        self.glPanel = wx.Panel(id=wxID_X3DEDITFRAMEGLPANEL, name=u'glPanel',
              parent=self.mainSplitterWindow, pos=wx.Point(0, 0),
              size=wx.Size(200, 525), style=wx.TAB_TRAVERSAL)

        self.detailsScrolledWindow = wx.ScrolledWindow(id=wxID_X3DEDITFRAMEDETAILSSCROLLEDWINDOW,
              name=u'detailsScrolledWindow', parent=self.mainSplitterWindow,
              pos=wx.Point(205, 0), size=wx.Size(725, 525),
              style=wx.HSCROLL | wx.VSCROLL)
        self.mainSplitterWindow.SplitVertically(self.glPanel,
              self.detailsScrolledWindow, 200)

        self.detailsTreeCtrl = wx.TreeCtrl(id=wxID_X3DEDITFRAMEDETAILSTREECTRL,
              name=u'detailsTreeCtrl', parent=self.detailsScrolledWindow,
              pos=wx.Point(0, 0), size=wx.Size(725, 525),
              style=wx.TR_HAS_BUTTONS)
        self.detailsTreeCtrl.SetAutoLayout(True)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)

        self.canvas = x3dCanvas.X3dCanvas(self)
        self.glSizer.AddWindow(self.canvas, 1, border=0, flag=wx.EXPAND)

    def OnRightDown(self,event):
        d = wx.FileDialog(self,wildcard='*.x3d',style=wx.OPEN|wx.HIDE_READONLY)
        ret = d.ShowModal()
        if ret == wx.ID_OK:
            self.sceneManager.addX3D().Load(gz4d.Filename(str(d.GetPath())))
