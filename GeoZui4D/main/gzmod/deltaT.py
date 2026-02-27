#!/usr/bin/env python
# gz
# DeltaT
# Adjust RT DeltaT display parameters

import wx
import deltaT
import gz4d
import math

class DeltaTFrame(deltaT.DeltaTFrameBase):
    def __init__(self, parent):
        deltaT.DeltaTFrameBase.__init__(self,parent)
        self.mod = False

        self.gz = gz4d.Connection()
        self.gz.Connect()

        s = float(self.gz.SendCommand('gz tellObject deltat elementSpacing')[1])
        self.spacingSlider.SetValue(s*1000000)
        self.spacingText.SetValue(str(s))

        bc = int(self.gz.SendCommand('gz tellObject deltat beamCount')[1])
        self.beamCountSlider.SetValue(bc)
        self.beamCountText.SetValue(str(bc))

        fr = float(self.gz.SendCommand('gz tellObject deltat frequencyRatio')[1])
        self.frequencyRatioSlider.SetValue(fr*10)
        self.frequencyRatioText.SetValue(str(fr))


        sa = math.degrees(float(self.gz.SendCommand('gz tellObject deltat swathAngle')[1]))
        self.swathAngleSlider.SetValue(sa)
        self.swathAngleText.SetValue(str(sa))

        e = int(self.gz.SendCommand('gz tellObject deltat showRaw ')[1])
        self.showRawElementDataCheckbox.SetValue(e)

        e = int(self.gz.SendCommand('gz tellObject deltat showRawPhase ')[1])
        self.showRawPhaseCheckbox.SetValue(e)

        e = int(self.gz.SendCommand('gz tellObject deltat showRawBits ')[1])
        self.showRawBitsCheckbox.SetValue(e)

        e = int(self.gz.SendCommand('gz tellObject deltat showBeamformedRectangle ')[1])
        self.showBeamformedRectangleCheckbox.SetValue(e)

        e = int(self.gz.SendCommand('gz tellObject deltat flipElements ')[1])
        self.flipElementsCheckbox.SetValue(e)

    def OnElementSpacingSlider(self, evt):
        if not self.mod:
            spacing = self.spacingSlider.GetValue()/1000000.0
            self.mod = True
            self.spacingText.SetValue(str(spacing))
            self.mod = False
            wx.Yield()
            self.gz.SendCommand('gz tellObject deltat elementSpacing '+str(spacing))

    def OnElementSpacingText(self, evt):
        if not self.mod:
            spacing = float(self.spacingText.GetValue())
            self.mod = True
            self.spacingSlider.SetValue(spacing*1000000)
            self.mod = False
            self.gz.SendCommand('gz tellObject deltat elementSpacing '+str(spacing))


    def OnFrequencyRatioSlider(self, evt):
        if not self.mod:
            ratio = self.frequencyRatioSlider.GetValue()/10.0
            self.mod = True
            self.frequencyRatioText.SetValue(str(ratio))
            self.mod = False
            wx.Yield()
            self.gz.SendCommand('gz tellObject deltat frequencyRatio '+str(ratio))

    def OnFrequencyRatioText(self, evt):
        if not self.mod:
            ratio = float(self.frequencyRatioText.GetValue())
            self.mod = True
            self.frequencyRatioSlider.SetValue(ratio*10)
            self.mod = False
            self.gz.SendCommand('gz tellObject deltat frequencyRatio '+str(ratio))

    def OnSwathAngleSlider(self, evt):
        if not self.mod:
            swath = self.swathAngleSlider.GetValue()
            self.mod = True
            self.swathAngleText.SetValue(str(swath))
            self.mod = False
            wx.Yield()
            self.gz.SendCommand('gz tellObject deltat swathAngle '+str(math.radians(swath)))

    def OnSwathAngleText(self, evt):
        if not self.mod:
            swath = float(self.swathAngleText.GetValue())
            self.mod = True
            self.swathAngleSlider.SetValue(swath)
            self.mod = False
            self.gz.SendCommand('gz tellObject deltat swathAngle '+str(math.radians(swath)))

    def OnBeamCountSlider(self, evt):
        if not self.mod:
            bc = self.beamCountSlider.GetValue()
            self.mod = True
            self.beamCountText.SetValue(str(bc))
            self.mod = False
            wx.Yield()
            self.gz.SendCommand('gz tellObject deltat beamCount '+str(bc))

    def OnBeamCountText(self, evt):
        if not self.mod:
            try:
                bc = int(self.beamCountText.GetValue())
            except ValueError:
                pass
            else:
                self.mod = True
                self.beamCountSlider.SetValue(bc)
                self.mod = False
                self.gz.SendCommand('gz tellObject deltat beamCount '+str(bc))

    def OnShowRawCheckbox(self, evt):
        self.gz.SendCommand('gz tellObject deltat showRaw '+str(self.showRawElementDataCheckbox.GetValue()))

    def OnShowRawPhaseCheckbox(self, evt):
        self.gz.SendCommand('gz tellObject deltat showRawPhase '+str(self.showRawPhaseCheckbox.GetValue()))

    def OnShowRawBitsCheckbox(self, evt):
        self.gz.SendCommand('gz tellObject deltat showRawBits '+str(self.showRawBitsCheckbox.GetValue()))

    def OnFlipElementsCheckbox(self, evt):
        self.gz.SendCommand('gz tellObject deltat flipElements '+str(self.flipElementsCheckbox.GetValue()))

    def OnShowBeamformedRectangleCheckbox(self, evt):
        self.gz.SendCommand('gz tellObject deltat showBeamformedRectangle '+str(self.showBeamformedRectangleCheckbox.GetValue()))


class DeltaT(wx.App):
    def OnInit(self):
        f = DeltaTFrame(None)
        f.Show()
        self.SetTopWindow(f)
        return True

if __name__ == '__main__':
    app = DeltaT()
    app.MainLoop()
