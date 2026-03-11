# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

#Boa:Frame:simMbFishFrame

import wx
import os
import dataSource

def create(parent):
    return simMbFishFrame(parent)

[wxID_SIMMBFISHFRAME, wxID_SIMMBFISHFRAMEADDRESSLABEL, 
 wxID_SIMMBFISHFRAMEADDRESSTEXTCTRL, wxID_SIMMBFISHFRAMEBATHYMETRYLABEL, 
 wxID_SIMMBFISHFRAMEBATHYMETRYTEXTCTRL, 
 wxID_SIMMBFISHFRAMECHOOSEBATHYMETRYBUTTON, 
 wxID_SIMMBFISHFRAMECHOOSEDISTORTIONGRIDBUTTON, 
 wxID_SIMMBFISHFRAMECHOOSEMODELBUTTON, 
 wxID_SIMMBFISHFRAMECHOOSEPOSITIONSBUTTON, 
 wxID_SIMMBFISHFRAMECHOOSETARGETSBUTTON, 
 wxID_SIMMBFISHFRAMECHOOSEWATERCOLUMNBUTTOM, wxID_SIMMBFISHFRAMECONNECTBUTTON, 
 wxID_SIMMBFISHFRAMECONNECTIONSTATUSLABEL, 
 wxID_SIMMBFISHFRAMEDISTORTIONGRIDLABEL, 
 wxID_SIMMBFISHFRAMEDISTORTIONGRIDTEXTCTRL, wxID_SIMMBFISHFRAMEMODELLABEL, 
 wxID_SIMMBFISHFRAMEMODELTEXTCTRL, wxID_SIMMBFISHFRAMEOUTPUTTEXTCTRL, 
 wxID_SIMMBFISHFRAMEPORTLABEL, wxID_SIMMBFISHFRAMEPORTTEXTCTRL, 
 wxID_SIMMBFISHFRAMEPOSITIONSLABEL, wxID_SIMMBFISHFRAMEPOSITIONSTEXTCTRL, 
 wxID_SIMMBFISHFRAMESTARTBUTTON, wxID_SIMMBFISHFRAMETARGETSLABEL, 
 wxID_SIMMBFISHFRAMETARGETSTEXTCTRL, wxID_SIMMBFISHFRAMEWATERCOLUMNLABEL, 
 wxID_SIMMBFISHFRAMEWATERCOLUMNTEXTCTRL, 
] = [wx.NewId() for _init_ctrls in range(27)]

[wxID_SIMMBFISHFRAMESIMTIMER] = [wx.NewId() for _init_utils in range(1)]

class simMbFishFrame(wx.Frame):
    def _init_coll_controlGridSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.connectButton, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.addressLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.addressTextCtrl, 1, border=0, flag=wx.EXPAND)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.connectionStatusLabel, 0, border=0,
              flag=wx.ALIGN_CENTER_HORIZONTAL | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.portLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.portTextCtrl, 1, border=0, flag=wx.EXPAND)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.positionsLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.positionsTextCtrl, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.choosePositionsButton, 0, border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.modelLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.modelTextCtrl, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.chooseModelButton, 0, border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.targetsLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.targetsTextCtrl, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.chooseTargetsButton, 0, border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.waterColumnLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.waterColumnTextCtrl, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.chooseWaterColumnButtom, 0, border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.distortionGridLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.distortionGridTextCtrl, 0, border=0,
              flag=wx.EXPAND)
        parent.AddWindow(self.chooseDistortionGridButton, 0, border=0, flag=0)
        parent.AddSpacer(wx.Size(8, 8), border=0, flag=0)
        parent.AddWindow(self.bathymetryLabel, 0, border=0,
              flag=wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL)
        parent.AddWindow(self.bathymetryTextCtrl, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.chooseBathymetryButton, 0, border=0, flag=0)
        parent.AddWindow(self.startButton, 0, border=0, flag=wx.EXPAND)

    def _init_coll_mainSize_Items(self, parent):
        # generated method, don't edit

        parent.AddSizer(self.controlGridSizer, 0, border=0, flag=wx.EXPAND)
        parent.AddWindow(self.outputTextCtrl, 1, border=0, flag=wx.EXPAND)

    def _init_utils(self):
        # generated method, don't edit
        self.simTimer = wx.Timer(id=wxID_SIMMBFISHFRAMESIMTIMER, owner=self)
        self.Bind(wx.EVT_TIMER, self.OnSimTimer, id=wxID_SIMMBFISHFRAMESIMTIMER)

    def _init_sizers(self):
        # generated method, don't edit
        self.mainSize = wx.BoxSizer(orient=wx.VERTICAL)

        self.controlGridSizer = wx.FlexGridSizer(cols=4, hgap=0, rows=1, vgap=0)
        self.controlGridSizer.SetMinSize(wx.Size(250, 224))

        self._init_coll_mainSize_Items(self.mainSize)
        self._init_coll_controlGridSizer_Items(self.controlGridSizer)

        self.SetSizer(self.mainSize)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_SIMMBFISHFRAME, name=u'simMbFishFrame',
              parent=prnt, pos=wx.Point(490, 365), size=wx.Size(800, 600),
              style=wx.DEFAULT_FRAME_STYLE, title=u'Sim MbFish')
        self._init_utils()
        self.SetClientSize(wx.Size(800, 600))

        self.connectButton = wx.Button(id=wxID_SIMMBFISHFRAMECONNECTBUTTON,
              label=u'Connect', name=u'connectButton', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(120, 28), style=0)
        self.connectButton.Bind(wx.EVT_BUTTON, self.OnConnectButton,
              id=wxID_SIMMBFISHFRAMECONNECTBUTTON)

        self.outputTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEOUTPUTTEXTCTRL,
              name=u'outputTextCtrl', parent=self, pos=wx.Point(0, 252),
              size=wx.Size(900, 600), style=wx.TE_READONLY | wx.TE_MULTILINE,
              value=u'')
        self.outputTextCtrl.SetAutoLayout(False)

        self.addressLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEADDRESSLABEL,
              label=u'Address:', name=u'addressLabel', parent=self,
              pos=wx.Point(160, 6), size=wx.Size(64, 15), style=wx.ALIGN_RIGHT)

        self.addressTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEADDRESSTEXTCTRL,
              name=u'addressTextCtrl', parent=self, pos=wx.Point(224, 0),
              size=wx.Size(158, 28), style=0, value=u'penguin.ccom.nh')

        self.portLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEPORTLABEL,
              label=u'Port:', name=u'portLabel', parent=self, pos=wx.Point(184,
              34), size=wx.Size(40, 15), style=wx.ALIGN_RIGHT)

        self.portTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEPORTTEXTCTRL,
              name=u'portTextCtrl', parent=self, pos=wx.Point(224, 28),
              size=wx.Size(158, 28), style=0, value=u'1974')

        self.positionsLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEPOSITIONSLABEL,
              label=u'Positions:', name=u'positionsLabel', parent=self,
              pos=wx.Point(156, 62), size=wx.Size(68, 15),
              style=wx.ALIGN_RIGHT)

        self.positionsTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEPOSITIONSTEXTCTRL,
              name=u'positionsTextCtrl', parent=self, pos=wx.Point(224, 56),
              size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/transect11/positions.gzx')

        self.choosePositionsButton = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSEPOSITIONSBUTTON,
              label=u'...', name=u'choosePositionsButton', parent=self,
              pos=wx.Point(382, 56), size=wx.Size(45, 28), style=0)
        self.choosePositionsButton.Bind(wx.EVT_BUTTON,
              self.OnChoosePositionsButton,
              id=wxID_SIMMBFISHFRAMECHOOSEPOSITIONSBUTTON)

        self.chooseModelButton = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSEMODELBUTTON,
              label=u'...', name=u'chooseModelButton', parent=self,
              pos=wx.Point(382, 84), size=wx.Size(45, 28), style=0)
        self.chooseModelButton.Bind(wx.EVT_BUTTON, self.OnChooseModelButton,
              id=wxID_SIMMBFISHFRAMECHOOSEMODELBUTTON)

        self.modelTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEMODELTEXTCTRL,
              name=u'modelTextCtrl', parent=self, pos=wx.Point(224, 84),
              size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/CoastalSurveyor04.dxf')

        self.startButton = wx.Button(id=wxID_SIMMBFISHFRAMESTARTBUTTON,
              label=u'Start', name=u'startButton', parent=self, pos=wx.Point(0,
              224), size=wx.Size(120, 28), style=0)
        self.startButton.Bind(wx.EVT_BUTTON, self.OnStartButton,
              id=wxID_SIMMBFISHFRAMESTARTBUTTON)

        self.chooseTargetsButton = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSETARGETSBUTTON,
              label=u'...', name=u'chooseTargetsButton', parent=self,
              pos=wx.Point(382, 112), size=wx.Size(45, 28), style=0)
        self.chooseTargetsButton.Bind(wx.EVT_BUTTON, self.OnChooseTargetsButton,
              id=wxID_SIMMBFISHFRAMECHOOSETARGETSBUTTON)

        self.targetsLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMETARGETSLABEL,
              label=u'Targets:', name=u'targetsLabel', parent=self,
              pos=wx.Point(168, 118), size=wx.Size(56, 15),
              style=wx.ALIGN_RIGHT)

        self.modelLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEMODELLABEL,
              label=u'Model:', name=u'modelLabel', parent=self,
              pos=wx.Point(182, 90), size=wx.Size(42, 15),
              style=wx.ALIGN_RIGHT)

        self.targetsTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMETARGETSTEXTCTRL,
              name=u'targetsTextCtrl', parent=self, pos=wx.Point(224, 112),
              size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/transect11/fish.nc')

        self.chooseWaterColumnButtom = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSEWATERCOLUMNBUTTOM,
              label=u'...', name=u'chooseWaterColumnButtom', parent=self,
              pos=wx.Point(382, 140), size=wx.Size(45, 28), style=0)
        self.chooseWaterColumnButtom.Bind(wx.EVT_BUTTON,
              self.OnChooseWaterColumnButton,
              id=wxID_SIMMBFISHFRAMECHOOSEWATERCOLUMNBUTTOM)

        self.chooseBathymetryButton = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSEBATHYMETRYBUTTON,
              label=u'...', name=u'chooseBathymetryButton', parent=self,
              pos=wx.Point(382, 196), size=wx.Size(45, 28), style=0)
        self.chooseBathymetryButton.Bind(wx.EVT_BUTTON,
              self.OnChooseBathymetryButton,
              id=wxID_SIMMBFISHFRAMECHOOSEBATHYMETRYBUTTON)

        self.waterColumnLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEWATERCOLUMNLABEL,
              label=u'Water Column:', name=u'waterColumnLabel', parent=self,
              pos=wx.Point(120, 146), size=wx.Size(104, 15),
              style=wx.ALIGN_RIGHT)

        self.bathymetryLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEBATHYMETRYLABEL,
              label=u'Bathymetry:', name=u'bathymetryLabel', parent=self,
              pos=wx.Point(150, 202), size=wx.Size(74, 15),
              style=wx.ALIGN_RIGHT)

        self.bathymetryTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEBATHYMETRYTEXTCTRL,
              name=u'bathymetryTextCtrl', parent=self, pos=wx.Point(224, 196),
              size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/transect11/bathymetry.nc')

        self.waterColumnTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEWATERCOLUMNTEXTCTRL,
              name=u'waterColumnTextCtrl', parent=self, pos=wx.Point(224, 140),
              size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/transect11/sftexture.nc')

        self.connectionStatusLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMECONNECTIONSTATUSLABEL,
              label=u'Not connected', name=u'connectionStatusLabel',
              parent=self, pos=wx.Point(0, 34), size=wx.Size(120, 15),
              style=wx.ALIGN_CENTRE)

        self.distortionGridLabel = wx.StaticText(id=wxID_SIMMBFISHFRAMEDISTORTIONGRIDLABEL,
              label=u'Distortion Grid:', name=u'distortionGridLabel',
              parent=self, pos=wx.Point(120, 174), size=wx.Size(104, 15),
              style=wx.ALIGN_RIGHT)

        self.distortionGridTextCtrl = wx.TextCtrl(id=wxID_SIMMBFISHFRAMEDISTORTIONGRIDTEXTCTRL,
              name=u'distortionGridTextCtrl', parent=self, pos=wx.Point(224,
              168), size=wx.Size(158, 28), style=0,
              value=u'/home/roland/work/ooa/transect11/sfgrid.nc')

        self.chooseDistortionGridButton = wx.Button(id=wxID_SIMMBFISHFRAMECHOOSEDISTORTIONGRIDBUTTON,
              label=u'...', name=u'chooseDistortionGridButton', parent=self,
              pos=wx.Point(382, 168), size=wx.Size(45, 28), style=0)
        self.chooseDistortionGridButton.Bind(wx.EVT_BUTTON,
              self.OnChooseDistortionGridButton,
              id=wxID_SIMMBFISHFRAMECHOOSEDISTORTIONGRIDBUTTON)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.controlGridSizer.AddGrowableCol(2,1)
        self.lastDir = ''
        self.data = dataSource.SourceManager()
        

    def OnConnectButton(self, event):
        self.outputTextCtrl.AppendText('Connecting...\n')
        self.data.Connect(self.addressTextCtrl.GetValue(),self.portTextCtrl.GetValue())
        self.connectionStatusLabel.SetLabel('Connected')

    def Choose(self,ctrl,wildcard,msg):
        d = wx.FileDialog(self,defaultDir=self.lastDir,style=wx.FD_OPEN,wildcard=wildcard,message=msg)
        if d.ShowModal() == wx.ID_OK:
            result = d.GetPath()
            ctrl.SetValue(result)
            self.lastDir = os.path.split(result)[0]
            return result

    def InitSource(self,source,fname):
        self.data.Init(source,fname)
            

    def OnStartButton(self, event):
        self.outputTextCtrl.AppendText('Starting...\n')
        self.data.Start({'positions':self.positionsTextCtrl.GetValue(),'bathymetry':self.bathymetryTextCtrl.GetValue(),'targets':self.targetsTextCtrl.GetValue(),'image':self.waterColumnTextCtrl.GetValue(),'grid':self.distortionGridTextCtrl.GetValue()})
        self.progress = wx.ProgressDialog(title='Simulation',message='Replaying data...',parent=self,style=wx.PD_APP_MODAL|wx.PD_AUTO_HIDE|wx.PD_CAN_ABORT|wx.PD_ELAPSED_TIME|wx.PD_ESTIMATED_TIME|wx.PD_REMAINING_TIME)
        self.simTimer.Start(1,oneShot=True)

    def OnSimTimer(self, event):
        
        if self.progress.Update(self.data.GetProgress())[0]:
            dt = self.data.Idle()
            self.simTimer.Start(self.data.Idle(),oneShot=True)
        else:
            self.progress.Destroy()
            self.progress = None


    def OnChoosePositionsButton(self, event):
        self.InitSource('positions',self.Choose(self.positionsTextCtrl,'GeoZui4D (*.gzx)|*.gzx','Select position source'))
        if self.modelTextCtrl.GetValue() == '':
            try:
                self.modelTextCtrl.SetValue(self.data.sources['model'].fname)
            except KeyError:
                pass

    def OnChooseModelButton(self, event):
        self.data.Init('model',self.Choose(self.modelTextCtrl,'X3D (*.x3d)|*.x3d|DXF (*.dxf)|*.dxf|All (*)|*','Select model'))

    def OnChooseWaterColumnButton(self, event):
        self.data.Init('image',self.Choose(self.waterColumnTextCtrl,'NetCDF (*.nc)|*.nc','Select water column imagery'))

    def OnChooseTargetsButton(self, event):
        self.data.Init('targets',self.Choose(self.targetsTextCtrl,'NetCDF (*.nc)|*.nc','Select targets'))
        self.outputTextCtrl.AppendText(str(self.data.endTime-self.data.startTime)+' seconds from '+str(self.data.startTime)+' to '+str(self.data.endTime)+'\n')

    def OnChooseBathymetryButton(self, event):
        self.data.Init('bathymetry',self.Choose(self.bathymetryTextCtrl,'NetCDF (*.nc)|*.nc','Select bathymetry source'))
        self.outputTextCtrl.AppendText(str(self.data.endTime-self.data.startTime)+' seconds from '+str(self.data.startTime)+' to '+str(self.data.endTime)+'\n')

    def OnChooseDistortionGridButton(self, event):
        self.data.Init('grid',self.Choose(self.distortionGridTextCtrl,'NetCDF (*.nc)|*.nc','Select grid source'))




