#Boa:Frame:BeamPatternFrame

import wx

import gz4d
import threading
import math

def create(parent):
    return BeamPatternFrame(parent)

[wxID_BEAMPATTERNFRAME, wxID_BEAMPATTERNFRAMEAMPLITUDELABEL, 
 wxID_BEAMPATTERNFRAMEAMPLITUDESLIDER, 
 wxID_BEAMPATTERNFRAMEAMPLITUDEVALUELABEL, wxID_BEAMPATTERNFRAMECOUNTLABEL, 
 wxID_BEAMPATTERNFRAMECOUNTVALUELABEL, wxID_BEAMPATTERNFRAMEDLABEL, 
 wxID_BEAMPATTERNFRAMEELEMENTTYPECHOICE, wxID_BEAMPATTERNFRAMELLABEL, 
 wxID_BEAMPATTERNFRAMEMINDBLABEL, wxID_BEAMPATTERNFRAMEMINDBSLIDER, 
 wxID_BEAMPATTERNFRAMEMINDBVALUELABEL, wxID_BEAMPATTERNFRAMENLABEL, 
 wxID_BEAMPATTERNFRAMEPHIRESOLUTIONLABEL, 
 wxID_BEAMPATTERNFRAMEPHIRESOLUTIONSLIDER, 
 wxID_BEAMPATTERNFRAMEPHIRESOLUTIONVALUELABEL, 
 wxID_BEAMPATTERNFRAMEPHISPANLABEL, wxID_BEAMPATTERNFRAMEPHISPANSLIDER, 
 wxID_BEAMPATTERNFRAMEPHISPANVALUELABEL, wxID_BEAMPATTERNFRAMEPHISTARTLABEL, 
 wxID_BEAMPATTERNFRAMEPHISTARTSLIDER, wxID_BEAMPATTERNFRAMEPHISTARTVALUELABEL, 
 wxID_BEAMPATTERNFRAMESIZELABEL, wxID_BEAMPATTERNFRAMESIZEVALUELABEL, 
 wxID_BEAMPATTERNFRAMESLABEL, wxID_BEAMPATTERNFRAMESPACINGLABEL, 
 wxID_BEAMPATTERNFRAMESPACINGVALUELABEL, 
 wxID_BEAMPATTERNFRAMETHETARESOLUTIONLABEL, 
 wxID_BEAMPATTERNFRAMETHETARESOLUTIONSLIDER, 
 wxID_BEAMPATTERNFRAMETHETARESOLUTIONVALUELABEL, 
 wxID_BEAMPATTERNFRAMETHETASPANLABEL, wxID_BEAMPATTERNFRAMETHETASPANSLIDER, 
 wxID_BEAMPATTERNFRAMETHETASPANVALUELABEL, 
 wxID_BEAMPATTERNFRAMETHETASTARTLABEL, wxID_BEAMPATTERNFRAMETHETASTARTSLIDER, 
 wxID_BEAMPATTERNFRAMETHETASTARTVALUELABEL, wxID_BEAMPATTERNFRAMETYPELABEL, 
 wxID_BEAMPATTERNFRAMEXCOUNTSLIDER, wxID_BEAMPATTERNFRAMEXSIZESLIDER, 
 wxID_BEAMPATTERNFRAMEXSPACINGSLIDER, wxID_BEAMPATTERNFRAMEXSTEERINGSLIDER, 
 wxID_BEAMPATTERNFRAMEYCOUNTSLIDER, wxID_BEAMPATTERNFRAMEYSIZESLIDER, 
 wxID_BEAMPATTERNFRAMEYSPACINGSLIDER, wxID_BEAMPATTERNFRAMEYSTEERINGSLIDER, 
] = [wx.NewId() for _init_ctrls in range(45)]

class BeamPatternFrame(wx.Frame):
    def _init_coll_mainSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.elementTypeChoice, (6, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.yCountSlider, (1, 0), border=0, flag=wx.EXPAND,
              span=(16, 1))
        parent.AddWindow(self.ySpacingSlider, (2, 1), border=0, flag=wx.EXPAND,
              span=(15, 1))
        parent.AddWindow(self.ySizeSlider, (3, 2), border=0, flag=wx.EXPAND,
              span=(14, 1))
        parent.AddWindow(self.xSizeSlider, (2, 3), border=0, flag=wx.EXPAND,
              span=(1, 3))
        parent.AddWindow(self.xSpacingSlider, (1, 2), border=0, flag=wx.EXPAND,
              span=(1, 4))
        parent.AddWindow(self.xCountSlider, (0, 1), border=0, flag=wx.EXPAND,
              span=(1, 5))
        parent.AddWindow(self.sizeLabel, (3, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.spacingLabel, (4, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.countLabel, (5, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.amplitudeLabel, (7, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.amplitudeSlider, (7, 5), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.amplitudeValueLabel, (7, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.sizeValueLabel, (3, 4), border=0, flag=wx.EXPAND,
              span=(1, 2))
        parent.AddWindow(self.spacingValueLabel, (4, 4), border=0,
              flag=wx.EXPAND, span=(1, 2))
        parent.AddWindow(self.countValueLabel, (5, 4), border=0, flag=wx.EXPAND,
              span=(1, 2))
        parent.AddWindow(self.typeLabel, (6, 3), border=0,
              flag=wx.SHAPED | wx.EXPAND | wx.ALIGN_CENTER, span=(1, 1))
        parent.AddWindow(self.thetaResolutionLabel, (9, 3), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaResolutionValueLabel, (9, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaResolutionSlider, (9, 5), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiResolutionLabel, (10, 3), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiResolutionValueLabel, (10, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiResolutionSlider, (10, 5), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.nLabel, (0, 0), border=0, flag=wx.EXPAND, span=(1,
              1))
        parent.AddWindow(self.dLabel, (1, 1), border=0, flag=wx.EXPAND, span=(1,
              1))
        parent.AddWindow(self.lLabel, (2, 2), border=0, flag=wx.EXPAND, span=(1,
              1))
        parent.AddWindow(self.thetaStartLabel, (11, 3), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaStartValueLabel, (11, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaStartSlider, (11, 5), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaSpanLabel, (12, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.thetaSpanValueLabel, (12, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.thetaSpanSlider, (12, 5), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiStartLabel, (13, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.phiStartValueLabel, (13, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiStartSlider, (13, 5), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.phiSpanLabel, (14, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.phiSpanValueLabel, (14, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.phiSpanSlider, (14, 5), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.minDBLabel, (15, 3), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.minDBValueLabel, (15, 4), border=0,
              flag=wx.EXPAND, span=(1, 1))
        parent.AddWindow(self.minDBSlider, (15, 5), border=0, flag=wx.EXPAND,
              span=(1, 1))
        parent.AddWindow(self.ySteeringSlider, (0, 6), border=0, flag=wx.EXPAND,
              span=(16, 1))
        parent.AddWindow(self.xSteeringSlider, (16, 3), border=0,
              flag=wx.EXPAND, span=(1, 3))
        parent.AddWindow(self.sLabel, (16, 6), border=0, flag=wx.EXPAND,
              span=(1, 1))

    def _init_coll_mainSizer_Growables(self, parent):
        # generated method, don't edit

        parent.AddGrowableRow(8)
        parent.AddGrowableCol(5)

    def _init_sizers(self):
        # generated method, don't edit
        self.mainSizer = wx.GridBagSizer(hgap=5, vgap=5)
        self.mainSizer.SetNonFlexibleGrowMode(wx.FLEX_GROWMODE_SPECIFIED)

        self._init_coll_mainSizer_Items(self.mainSizer)
        self._init_coll_mainSizer_Growables(self.mainSizer)

        self.SetSizer(self.mainSizer)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_BEAMPATTERNFRAME,
              name=u'BeamPatternFrame', parent=prnt, pos=wx.Point(345, 331),
              size=wx.Size(992, 585), style=wx.DEFAULT_FRAME_STYLE,
              title=u'Beam Pattern')
        self.SetClientSize(wx.Size(992, 585))
        self.SetAutoLayout(False)
        self.Bind(wx.EVT_CLOSE, self.OnClose)

        self.yCountSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEYCOUNTSLIDER,
              maxValue=100, minValue=1, name=u'yCountSlider', parent=self,
              style=wx.SL_VERTICAL, value=1)
        self.yCountSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged,
              id=wxID_BEAMPATTERNFRAMEYCOUNTSLIDER)

        self.ySpacingSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEYSPACINGSLIDER,
              maxValue=1000, minValue=1, name=u'ySpacingSlider', parent=self,
              style=wx.SL_VERTICAL, value=100)
        self.ySpacingSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged,
              id=wxID_BEAMPATTERNFRAMEYSPACINGSLIDER)

        self.ySizeSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEYSIZESLIDER,
              maxValue=1000, minValue=1, name=u'ySizeSlider', parent=self,
              style=wx.SL_VERTICAL, value=50)
        self.ySizeSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.xCountSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEXCOUNTSLIDER,
              maxValue=100, minValue=1, name=u'xCountSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=1)
        self.xCountSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.xSpacingSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEXSPACINGSLIDER,
              maxValue=1000, minValue=1, name=u'xSpacingSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=100)
        self.xSpacingSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.xSizeSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEXSIZESLIDER,
              maxValue=1000, minValue=1, name=u'xSizeSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=50)
        self.xSizeSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.elementTypeChoice = wx.Choice(choices=['Point', 'Line',
              'Rectangular', 'Circular'],
              id=wxID_BEAMPATTERNFRAMEELEMENTTYPECHOICE,
              name=u'elementTypeChoice', parent=self, style=0)
        self.elementTypeChoice.SetSelection(2)
        self.elementTypeChoice.Bind(wx.EVT_CHOICE, self.OnValueChanged,
              id=wxID_BEAMPATTERNFRAMEELEMENTTYPECHOICE)

        self.spacingLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMESPACINGLABEL,
              label=u'Spacing:', name=u'spacingLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.countLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMECOUNTLABEL,
              label=u'Count:', name=u'countLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.sizeLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMESIZELABEL,
              label=u'Size:', name=u'sizeLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.amplitudeLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEAMPLITUDELABEL,
              label=u'Amplitude:', name=u'amplitudeLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.amplitudeSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEAMPLITUDESLIDER,
              maxValue=1000, minValue=0, name=u'amplitudeSlider', parent=self,
              pos=wx.Point(317, 184), style=wx.SL_HORIZONTAL, value=100)
        self.amplitudeSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.amplitudeValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEAMPLITUDEVALUELABEL,
              label=u'', name=u'amplitudeValueLabel', parent=self, style=0)

        self.sizeValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMESIZEVALUELABEL,
              label=u'', name=u'sizeValueLabel', parent=self, style=0)

        self.spacingValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMESPACINGVALUELABEL,
              label=u'', name=u'spacingValueLabel', parent=self, style=0)

        self.countValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMECOUNTVALUELABEL,
              label=u'', name=u'countValueLabel', parent=self, style=0)

        self.typeLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETYPELABEL,
              label=u'Element type:', name=u'typeLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.thetaResolutionLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETARESOLUTIONLABEL,
              label=u'Theta resolution:', name=u'thetaResolutionLabel',
              parent=self, pos=wx.Point(72, 304), style=wx.ALIGN_RIGHT)

        self.phiResolutionLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHIRESOLUTIONLABEL,
              label=u'Phi resolution:', name=u'phiResolutionLabel', parent=self,
              pos=wx.Point(72, 329), style=wx.ALIGN_RIGHT)

        self.thetaResolutionValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETARESOLUTIONVALUELABEL,
              label=u'', name=u'thetaResolutionValueLabel', parent=self,
              pos=wx.Point(202, 304), style=0)

        self.phiResolutionValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHIRESOLUTIONVALUELABEL,
              label=u'', name=u'phiResolutionValueLabel', parent=self,
              pos=wx.Point(202, 329), style=0)

        self.thetaResolutionSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMETHETARESOLUTIONSLIDER,
              maxValue=100, minValue=1, name=u'thetaResolutionSlider',
              parent=self, style=wx.SL_HORIZONTAL, value=10)
        self.thetaResolutionSlider.SetLabel(u'')
        self.thetaResolutionSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.phiResolutionSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEPHIRESOLUTIONSLIDER,
              maxValue=100, minValue=1, name=u'phiResolutionSlider',
              parent=self, style=wx.SL_HORIZONTAL, value=15)
        self.phiResolutionSlider.SetLabel(u'')
        self.phiResolutionSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.nLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMENLABEL, label=u'N',
              name=u'nLabel', parent=self, style=wx.ALIGN_CENTRE)

        self.dLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEDLABEL, label=u'D',
              name=u'dLabel', parent=self, style=wx.ALIGN_CENTRE)

        self.lLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMELLABEL, label=u'L',
              name=u'lLabel', parent=self, pos=wx.Point(50, 50),
              style=wx.ALIGN_CENTRE)

        self.thetaStartLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETASTARTLABEL,
              label=u'Theta start:', name=u'thetaStartLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.thetaStartValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETASTARTVALUELABEL,
              label=u'', name=u'thetaStartValueLabel', parent=self, style=0)

        self.thetaStartSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMETHETASTARTSLIDER,
              maxValue=100, minValue=0, name=u'thetaStartSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=0)
        self.thetaStartSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.thetaSpanLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETASPANLABEL,
              label=u'Theta span:', name=u'thetaSpanLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.thetaSpanValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMETHETASPANVALUELABEL,
              label=u'', name=u'thetaSpanValueLabel', parent=self, style=0)

        self.thetaSpanSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMETHETASPANSLIDER,
              maxValue=100, minValue=0, name=u'thetaSpanSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=50)
        self.thetaSpanSlider.SetLabel(u'')
        self.thetaSpanSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.phiStartLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHISTARTLABEL,
              label=u'Phi start:', name=u'phiStartLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.phiStartValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHISTARTVALUELABEL,
              label=u'', name=u'phiStartValueLabel', parent=self, style=0)

        self.phiStartSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEPHISTARTSLIDER,
              maxValue=100, minValue=-100, name=u'phiStartSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=0)
        self.phiStartSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.phiSpanLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHISPANLABEL,
              label=u'Phi span:', name=u'phiSpanLabel', parent=self,
              pos=wx.Point(72, 454), style=wx.ALIGN_RIGHT)

        self.phiSpanValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEPHISPANVALUELABEL,
              label=u'', name=u'phiSpanValueLabel', parent=self, style=0)

        self.phiSpanSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEPHISPANSLIDER,
              maxValue=200, minValue=0, name=u'phiSpanSlider', parent=self,
              pos=wx.Point(317, 429), style=wx.SL_HORIZONTAL, value=100)
        self.phiSpanSlider.SetLabel(u'')
        self.phiSpanSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.minDBLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEMINDBLABEL,
              label=u'Minimum dB:', name=u'minDBLabel', parent=self,
              style=wx.ALIGN_RIGHT)

        self.minDBValueLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMEMINDBVALUELABEL,
              label=u'', name=u'minDBValueLabel', parent=self, style=0)

        self.minDBSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEMINDBSLIDER,
              maxValue=20, minValue=1, name=u'minDBSlider', parent=self,
              style=wx.SL_HORIZONTAL, value=6)
        self.minDBSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.ySteeringSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEYSTEERINGSLIDER,
              maxValue=90, minValue=-90, name=u'ySteeringSlider', parent=self,
              style=wx.SL_VERTICAL, value=0)
        self.ySteeringSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.xSteeringSlider = wx.Slider(id=wxID_BEAMPATTERNFRAMEXSTEERINGSLIDER,
              maxValue=90, minValue=-90, name=u'xSteeringSlider', parent=self,
              pos=wx.Point(72, 565), size=wx.Size(904, 20),
              style=wx.SL_HORIZONTAL, value=0)
        self.xSteeringSlider.Bind(wx.EVT_SCROLL, self.OnValueChanged)

        self.sLabel = wx.StaticText(id=wxID_BEAMPATTERNFRAMESLABEL, label=u'S',
              name=u'sLabel', parent=self, style=wx.ALIGN_CENTRE)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)

        self.gz = gz4d.Connection()
        self.gz.Connect()
        self.obj = self.gz.SendCommand('gz newobject BeamPattern beamPattern')[1]

        self.gz.SendCommand('gz tellObject ' + self.obj + ' hud on')

        self.command = None
        self.done = False

        self.cmdCondition = threading.Condition()
        self.commThread = threading.Thread(target=self.CommThread)
        self.commThread.start()
        self.OnValueChanged()
        
    def CommThread(self):
        while(True):
            
            self.cmdCondition.acquire()
            if self.done:
                self.cmdCondition.release()
                return
            while not self.command:
                if self.done:
                    self.cmdCondition.release()
                    return
                self.cmdCondition.wait()
            cmd = self.command
            self.command = None
            self.cmdCondition.release()

            for c in cmd:
                self.gz.SendCommand(c)
                


    def OnValueChanged(self, event=None):
        elementType = self.elementTypeChoice.GetStringSelection()
        xSize = self.xSizeSlider.GetValue()/100.0
        ySize = self.ySizeSlider.GetValue()/100.0
        xSpacing = self.xSpacingSlider.GetValue()/100.0
        ySpacing = self.ySpacingSlider.GetValue()/100.0
        xCount = self.xCountSlider.GetValue()
        yCount = self.yCountSlider.GetValue()
        amplitude = self.amplitudeSlider.GetValue()/100.0
        
        self.sizeValueLabel.SetLabel(str(xSize)+' x '+ str(ySize)+ ' elements of type '+elementType)
        self.spacingValueLabel.SetLabel(str(xSpacing)+' x '+ str(ySpacing))
        self.countValueLabel.SetLabel(str(xCount)+' x '+ str(yCount))
        self.amplitudeValueLabel.SetLabel(str(amplitude))

        thetaRes = self.thetaResolutionSlider.GetValue()*5
        phiRes = self.phiResolutionSlider.GetValue()*5

        self.thetaResolutionValueLabel.SetLabel(str(thetaRes))
        self.phiResolutionValueLabel.SetLabel(str(phiRes))

        thetaStart = self.thetaStartSlider.GetValue()*0.01*math.pi
        thetaSpan = self.thetaSpanSlider.GetValue()*0.01*math.pi
        phiStart = self.phiStartSlider.GetValue()*0.01*math.pi
        phiSpan = self.phiSpanSlider.GetValue()*0.01*math.pi

        self.thetaStartValueLabel.SetLabel(str(thetaStart))
        self.thetaSpanValueLabel.SetLabel(str(thetaSpan))
        self.phiStartValueLabel.SetLabel(str(phiStart))
        self.phiSpanValueLabel.SetLabel(str(phiSpan))

        minDB = self.minDBSlider.GetValue()*(-5.0)
        self.minDBValueLabel.SetLabel(str(minDB)+' dB')

        xSteering = self.xSteeringSlider.GetValue()*math.pi/180.0
        ySteering = -self.ySteeringSlider.GetValue()*math.pi/180.0

        command = []
        command.append('gz tellObject ' + self.obj + ' setNM ' + str(xCount) + ' ' + str(yCount))
        command.append('gz tellObject ' + self.obj + ' setSpacing ' + str(xSpacing) + ' ' + str(ySpacing))
        command.append('gz tellObject ' + self.obj + ' setL ' + str(xSize) + ' ' + str(ySize))
        command.append('gz tellObject ' + self.obj + ' setType ' + elementType)
        command.append('gz tellObject ' + self.obj + ' setAmplitude ' + str(amplitude))
        command.append('gz tellObject ' + self.obj + ' setResolution ' + str(thetaRes) + ' ' + str(phiRes))
        command.append('gz tellObject ' + self.obj + ' setRange ' + str(thetaStart) + ' ' + str(thetaStart + thetaSpan) + ' ' + str(phiStart) + ' ' + str(phiStart + phiSpan))
        command.append('gz tellObject ' + self.obj + ' setMinDB ' + str(minDB))
        command.append('gz tellObject ' + self.obj + ' setSteering ' + str(xSteering) + ' ' + str(ySteering))
        command.append('gz tellObject ' + self.obj + ' update')
        
        self.cmdCondition.acquire()
        self.command = command
        self.cmdCondition.notify()
        self.cmdCondition.release()

    def OnClose(self, event):
        self.cmdCondition.acquire()
        self.done = True
        self.cmdCondition.notify()
        self.cmdCondition.release()
        self.commThread.join()
        self.Destroy()
        

