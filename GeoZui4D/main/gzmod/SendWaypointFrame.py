#Boa:Frame:SendWaypointFrame

import wx

import gz4d
import socket

def create(parent):
    return SendWaypointFrame(parent)

[wxID_SENDWAYPOINTFRAME, wxID_SENDWAYPOINTFRAMEABORTCIRCLELABEL, 
 wxID_SENDWAYPOINTFRAMEABORTCIRCLESPINCTRL, 
 wxID_SENDWAYPOINTFRAMECOMMANDTEXTCTRL, 
 wxID_SENDWAYPOINTFRAMEGETPOSITIONBUTTON, wxID_SENDWAYPOINTFRAMEGPSERRORLABEL, 
 wxID_SENDWAYPOINTFRAMEGPSERRSPINCTRL, wxID_SENDWAYPOINTFRAMEPOSITIONTEXTCTRL, 
 wxID_SENDWAYPOINTFRAMERPMLABEL, wxID_SENDWAYPOINTFRAMERPMSLIDER, 
 wxID_SENDWAYPOINTFRAMERPMTEXTCTRL, wxID_SENDWAYPOINTFRAMESENDWAYPOINTBUTTON, 
] = [wx.NewId() for _init_ctrls in range(12)]

class SendWaypointFrame(wx.Frame):
    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_SENDWAYPOINTFRAME,
              name=u'SendWaypointFrame', parent=prnt, pos=wx.Point(262, 434),
              size=wx.Size(832, 136), style=wx.DEFAULT_FRAME_STYLE,
              title=u'Send Waypoint')
        self.SetClientSize(wx.Size(832, 136))

        self.getPositionButton = wx.Button(id=wxID_SENDWAYPOINTFRAMEGETPOSITIONBUTTON,
              label=u'Get Position', name=u'getPositionButton', parent=self,
              pos=wx.Point(8, 16), size=wx.Size(128, 32), style=0)
        self.getPositionButton.Bind(wx.EVT_BUTTON,
              self.OnGetPositionButtonButton,
              id=wxID_SENDWAYPOINTFRAMEGETPOSITIONBUTTON)

        self.sendWaypointButton = wx.Button(id=wxID_SENDWAYPOINTFRAMESENDWAYPOINTBUTTON,
              label=u'Send Waypoint', name=u'sendWaypointButton', parent=self,
              pos=wx.Point(8, 88), size=wx.Size(128, 32), style=0)
        self.sendWaypointButton.Bind(wx.EVT_BUTTON,
              self.OnSendWaypointButtonButton,
              id=wxID_SENDWAYPOINTFRAMESENDWAYPOINTBUTTON)

        self.positionTextCtrl = wx.TextCtrl(id=wxID_SENDWAYPOINTFRAMEPOSITIONTEXTCTRL,
              name=u'positionTextCtrl', parent=self, pos=wx.Point(152, 16),
              size=wx.Size(664, 23), style=wx.TE_READONLY, value=u'')

        self.commandTextCtrl = wx.TextCtrl(id=wxID_SENDWAYPOINTFRAMECOMMANDTEXTCTRL,
              name=u'commandTextCtrl', parent=self, pos=wx.Point(152, 96),
              size=wx.Size(664, 23), style=wx.TE_READONLY, value=u'')

        self.gpsErrSpinCtrl = wx.SpinCtrl(id=wxID_SENDWAYPOINTFRAMEGPSERRSPINCTRL,
              initial=0, max=100, min=0, name=u'gpsErrSpinCtrl', parent=self,
              pos=wx.Point(152, 56), size=wx.Size(64, 23),
              style=wx.SP_ARROW_KEYS)

        self.gpsErrorLabel = wx.StaticText(id=wxID_SENDWAYPOINTFRAMEGPSERRORLABEL,
              label=u'GPS Error (m)', name=u'gpsErrorLabel', parent=self,
              pos=wx.Point(32, 64), size=wx.Size(85, 15), style=0)

        self.abortCircleSpinCtrl = wx.SpinCtrl(id=wxID_SENDWAYPOINTFRAMEABORTCIRCLESPINCTRL,
              initial=0, max=100, min=0, name=u'abortCircleSpinCtrl',
              parent=self, pos=wx.Point(368, 56), size=wx.Size(95, 23),
              style=wx.SP_ARROW_KEYS)

        self.abortCircleLabel = wx.StaticText(id=wxID_SENDWAYPOINTFRAMEABORTCIRCLELABEL,
              label=u'Abort Circle (m)', name=u'abortCircleLabel', parent=self,
              pos=wx.Point(264, 64), size=wx.Size(104, 16), style=0)

        self.rpmSlider = wx.Slider(id=wxID_SENDWAYPOINTFRAMERPMSLIDER,
              maxValue=100, minValue=0, name=u'rpmSlider', parent=self,
              point=wx.Point(616, 56), size=wx.Size(200, 20),
              style=wx.SL_HORIZONTAL, value=0)
        self.rpmSlider.Bind(wx.EVT_SCROLL, self.OnRpmSliderScroll)

        self.rpmLabel = wx.StaticText(id=wxID_SENDWAYPOINTFRAMERPMLABEL,
              label=u'RPM', name=u'rpmLabel', parent=self, pos=wx.Point(504,
              64), size=wx.Size(26, 15), style=0)

        self.rpmTextCtrl = wx.TextCtrl(id=wxID_SENDWAYPOINTFRAMERPMTEXTCTRL,
              name=u'rpmTextCtrl', parent=self, pos=wx.Point(544, 56),
              size=wx.Size(64, 23), style=wx.TE_READONLY, value=u'')

    def __init__(self, parent):
        self._init_ctrls(parent)

        self.gz = gz4d.Connection()
        self.gz.Connect()

        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.sock.bind(('',2065))
        
        self.gpsErrSpinCtrl.SetRange(5,150)
        self.gpsErrSpinCtrl.SetValue(15)

        self.abortCircleSpinCtrl.SetRange(500,5000)
        self.abortCircleSpinCtrl.SetValue(1500)
        
        self.rpmSlider.SetValue(35)


    def OnGetPositionButtonButton(self, event):
        result = self.gz.SendCommand('gz posable get Window.main xy')
        #print result
        if result[0]:
            parts = result[1].split()
            e = float(parts[0])
            n = float(parts[1])
            print e,n
            ll = self.gz.SendCommand(' '.join(('gz projection invert UTM.ZONE36',str(e),str(n))))
            if ll[0]:
                #print ll[1]
                parts = ll[1].split()
                self.lon = float(parts[0])
                self.lat = float(parts[1])
                #print self.lat, self.lon
                self.positionTextCtrl.SetValue(str(self.lat)+', '+str(self.lon))
                self.commandTextCtrl.SetValue('w'+str(self.lat)+', '+str(self.lon)+', '+str(self.gpsErrSpinCtrl.GetValue())+', '+str(self.abortCircleSpinCtrl.GetValue())+', '+self.rpmTextCtrl.GetValue()+'\n')

    def OnSendWaypointButtonButton(self, event):
        print self.commandTextCtrl.GetValue()
        self.sock.sendto(self.commandTextCtrl.GetValue(),('192.168.5.100',2065))

    def OnRpmSliderScroll(self, event):
        self.rpmTextCtrl.SetValue(str(self.rpmSlider.GetValue()*10))
