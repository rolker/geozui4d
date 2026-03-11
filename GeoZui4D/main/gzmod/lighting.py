#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# gz
# Lighting
# Adjust light levels

import wx
import lighting
import gz4d

GL_LINEAR = 0x2601
GL_EXP = 0x0800
GL_EXP2 = 0x0801

class LightingFrame(lighting.LightingFrameBase):
    def __init__(self, parent):
        lighting.LightingFrameBase.__init__(self,parent)

        self.gz = gz4d.Connection()
        self.gz.Connect()

        a = self.gz.SendCommand('gz tellObject Window.main.Light ambient')[1].split()
        self.ambient = [float(a[0]),float(a[1]),float(a[2]),float(a[3])]
        self.UpdateAmbient(False)

        d = self.gz.SendCommand('gz tellObject Window.main.Light diffuse')[1].split()
        self.diffuse = [float(d[0]),float(d[1]),float(d[2]),float(d[3])]
        self.drSlider.SetValue(int(self.diffuse[0]*255))
        self.dgSlider.SetValue(int(self.diffuse[1]*255))
        self.dbSlider.SetValue(int(self.diffuse[2]*255))
        self.daSlider.SetValue(int(self.diffuse[3]*255))

        s = self.gz.SendCommand('gz tellObject Window.main.Light specular')[1].split()
        self.specular = [float(s[0]),float(s[1]),float(s[2]),float(s[3])]
        self.srSlider.SetValue(int(self.specular[0]*255))
        self.sgSlider.SetValue(int(self.specular[1]*255))
        self.sbSlider.SetValue(int(self.specular[2]*255))
        self.saSlider.SetValue(int(self.specular[3]*255))

        f = self.gz.SendCommand('gz tellObject Window.main.Light fog color')[1].split()
        self.fogColor = [float(f[0]),float(f[1]),float(f[2]),float(f[3])]
        self.fogRSlider.SetValue(int(self.fogColor[0]*255))
        self.fogGSlider.SetValue(int(self.fogColor[1]*255))
        self.fogBSlider.SetValue(int(self.fogColor[2]*255))
        self.fogASlider.SetValue(int(self.fogColor[3]*255))

        f = int(self.gz.SendCommand('gz tellObject Window.main.Light fog mode')[1])
        print 'Mode:',f
        if f == GL_LINEAR:
            self.fogModeCombo.SetValue('Linear')
        if f == GL_EXP:
            self.fogModeCombo.SetValue('Exp')
        if f == GL_EXP2:
            self.fogModeCombo.SetValue('Exp2')

        f = self.gz.SendCommand('gz tellObject Window.main.Light fog start')[1]
        self.fogStartText.SetValue(f)

        f = self.gz.SendCommand('gz tellObject Window.main.Light fog end')[1]
        self.fogEndText.SetValue(f)

        f = self.gz.SendCommand('gz tellObject Window.main.Light fog density')[1]
        self.fogDensityText.SetValue(f)

    def UpdateAmbient(self,updateGZ = True):
        if self.ambient[0] < 0.0:
            self.ambient[0] = 0.0
        if self.ambient[0] > 1.0:
            self.ambient[0] = 1.0
        self.arSlider.SetValue(int(self.ambient[0]*255))
        if self.ambient[1] < 0.0:
            self.ambient[1] = 0.0
        if self.ambient[1] > 1.0:
            self.ambient[1] = 1.0
        self.agSlider.SetValue(int(self.ambient[1]*255))
        if self.ambient[2] < 0.0:
            self.ambient[2] = 0.0
        if self.ambient[2] > 1.0:
            self.ambient[2] = 1.0
        self.abSlider.SetValue(int(self.ambient[2]*255))
        if self.ambient[3] < 0.0:
            self.ambient[3] = 0.0
        if self.ambient[3] > 1.0:
            self.ambient[3] = 1.0
        self.aaSlider.SetValue(int(self.ambient[3]*255))
        if updateGZ:
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light ambient',str(self.ambient[0]),str(self.ambient[1]),str(self.ambient[2]),str(self.ambient[3]))))

    def OnAmbientR(self,evt):
        a = self.arSlider.GetValue()/255.0
        if self.ambientLockCheckbox.GetValue():
            r = a - self.ambient[0]
            self.ambient[1] += r
            self.ambient[2] += r
        self.ambient[0] = a
        self.UpdateAmbient()

    def OnAmbientG(self,evt):
        a = self.agSlider.GetValue()/255.0
        if self.ambientLockCheckbox.GetValue():
            r = a - self.ambient[1]
            self.ambient[0] += r
            self.ambient[2] += r
        self.ambient[1] = a
        self.UpdateAmbient()

    def OnAmbientB(self,evt):
        a = self.abSlider.GetValue()/255.0
        if self.ambientLockCheckbox.GetValue():
            r = a - self.ambient[2]
            self.ambient[0] += r
            self.ambient[1] += r
        self.ambient[2] = a
        self.UpdateAmbient()

    def OnAmbientA(self,evt):
        self.ambient[3] = self.aaSlider.GetValue()/255.0
        self.UpdateAmbient()

    def UpdateDiffuse(self,updateGZ = True):
        if self.diffuse[0] < 0.0:
            self.diffuse[0] = 0.0
        if self.diffuse[0] > 1.0:
            self.diffuse[0] = 1.0
        self.drSlider.SetValue(int(self.diffuse[0]*255))
        if self.diffuse[1] < 0.0:
            self.diffuse[1] = 0.0
        if self.diffuse[1] > 1.0:
            self.diffuse[1] = 1.0
        self.dgSlider.SetValue(int(self.diffuse[1]*255))
        if self.diffuse[2] < 0.0:
            self.diffuse[2] = 0.0
        if self.diffuse[2] > 1.0:
            self.diffuse[2] = 1.0
        self.dbSlider.SetValue(int(self.diffuse[2]*255))
        if self.diffuse[3] < 0.0:
            self.diffuse[3] = 0.0
        if self.diffuse[3] > 1.0:
            self.diffuse[3] = 1.0
        self.daSlider.SetValue(int(self.diffuse[3]*255))
        if updateGZ:
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light diffuse',str(self.diffuse[0]),str(self.diffuse[1]),str(self.diffuse[2]),str(self.diffuse[3]))))

    def OnDiffuseR(self,evt):
        a = self.drSlider.GetValue()/255.0
        if self.diffuseLockCheckbox.GetValue():
            r = a - self.diffuse[0]
            self.diffuse[1] += r
            self.diffuse[2] += r
        self.diffuse[0] = a
        self.UpdateDiffuse()

    def OnDiffuseG(self,evt):
        a = self.dgSlider.GetValue()/255.0
        if self.diffuseLockCheckbox.GetValue():
            r = a - self.diffuse[1]
            self.diffuse[0] += r
            self.diffuse[2] += r
        self.diffuse[1] = a
        self.UpdateDiffuse()

    def OnDiffuseB(self,evt):
        a = self.dbSlider.GetValue()/255.0
        if self.diffuseLockCheckbox.GetValue():
            r = a - self.diffuse[2]
            self.diffuse[0] += r
            self.diffuse[1] += r
        self.diffuse[2] = a
        self.UpdateDiffuse()

    def OnDiffuseA(self,evt):
        self.diffuse[3] = self.daSlider.GetValue()/255.0
        self.UpdateDiffuse()

    def UpdateSpecular(self,updateGZ = True):
        if self.specular[0] < 0.0:
            self.specular[0] = 0.0
        if self.specular[0] > 1.0:
            self.specular[0] = 1.0
        self.srSlider.SetValue(int(self.specular[0]*255))
        if self.specular[1] < 0.0:
            self.specular[1] = 0.0
        if self.specular[1] > 1.0:
            self.specular[1] = 1.0
        self.sgSlider.SetValue(int(self.specular[1]*255))
        if self.specular[2] < 0.0:
            self.specular[2] = 0.0
        if self.specular[2] > 1.0:
            self.specular[2] = 1.0
        self.sbSlider.SetValue(int(self.specular[2]*255))
        if self.specular[3] < 0.0:
            self.specular[3] = 0.0
        if self.specular[3] > 1.0:
            self.specular[3] = 1.0
        self.saSlider.SetValue(int(self.specular[3]*255))
        if updateGZ:
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light specular',str(self.specular[0]),str(self.specular[1]),str(self.specular[2]),str(self.specular[3]))))

    def OnSpecularR(self,evt):
        a = self.srSlider.GetValue()/255.0
        if self.specularLockCheckbox.GetValue():
            r = a - self.specular[0]
            self.specular[1] += r
            self.specular[2] += r
        self.specular[0] = a
        self.UpdateSpecular()

    def OnSpecularG(self,evt):
        a = self.sgSlider.GetValue()/255.0
        if self.specularLockCheckbox.GetValue():
            r = a - self.specular[1]
            self.specular[0] += r
            self.specular[2] += r
        self.specular[1] = a
        self.UpdateSpecular()

    def OnSpecularB(self,evt):
        a = self.sbSlider.GetValue()/255.0
        if self.specularLockCheckbox.GetValue():
            r = a - self.specular[2]
            self.specular[0] += r
            self.specular[1] += r
        self.specular[2] = a
        self.UpdateSpecular()

    def OnSpecularA(self,evt):
        self.specular[3] = self.aaSlider.GetValue()/255.0
        self.UpdateSpecular()


    def UpdateFogColor(self,updateGZ = True):
        if self.fogColor[0] < 0.0:
            self.fogColor[0] = 0.0
        if self.fogColor[0] > 1.0:
            self.fogColor[0] = 1.0
        self.fogRSlider.SetValue(int(self.fogColor[0]*255))
        if self.fogColor[1] < 0.0:
            self.fogColor[1] = 0.0
        if self.fogColor[1] > 1.0:
            self.fogColor[1] = 1.0
        self.fogGSlider.SetValue(int(self.fogColor[1]*255))
        if self.fogColor[2] < 0.0:
            self.fogColor[2] = 0.0
        if self.fogColor[2] > 1.0:
            self.fogColor[2] = 1.0
        self.fogBSlider.SetValue(int(self.fogColor[2]*255))
        if self.fogColor[3] < 0.0:
            self.fogColor[3] = 0.0
        if self.fogColor[3] > 1.0:
            self.fogColor[3] = 1.0
        self.fogASlider.SetValue(int(self.fogColor[3]*255))
        if updateGZ:
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog color',str(self.fogColor[0]),str(self.fogColor[1]),str(self.fogColor[2]),str(self.fogColor[3]))))

    def OnFogR(self,evt):
        a = self.fogRSlider.GetValue()/255.0
        self.fogColor[0] = a
        self.UpdateFogColor()

    def OnFogG(self,evt):
        a = self.fogGSlider.GetValue()/255.0
        self.fogColor[1] = a
        self.UpdateFogColor()

    def OnFogB(self,evt):
        a = self.fogBSlider.GetValue()/255.0
        self.fogColor[2] = a
        self.UpdateFogColor()

    def OnFogA(self,evt):
        self.fogColor[3] = self.fogASlider.GetValue()/255.0
        self.UpdateFogColor()

    def OnFogMode(self,evt):
        if self.fogModeCombo.GetValue() == 'Linear':
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog mode',str(GL_LINEAR))))
        if self.fogModeCombo.GetValue() == 'Exp':
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog mode',str(GL_EXP))))
        if self.fogModeCombo.GetValue() == 'Exp2':
            self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog mode',str(GL_EXP2))))

    def OnFogStart(self,evt):
        try:
            v = float(self.fogStartText.GetValue())
        except ValueError:
            return
        print 'start:',v
        self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog start',str(v))))

    def OnFogEnd(self,evt):
        try:
            v = float(self.fogEndText.GetValue())
        except ValueError:
            return
        print 'end:',v
        self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog end',str(v))))

    def OnFogDensity(self,evt):
        try:
            v = float(self.fogDensityText.GetValue())
        except ValueError:
            return
        print 'density:',v
        self.gz.SendCommand(' '.join(('gz tellObject Window.main.Light fog density',str(v))))

class Lighting(wx.App):
    def OnInit(self):
        f = LightingFrame(None)
        f.Show()
        self.SetTopWindow(f)
        return True

if __name__ == '__main__':
    app = Lighting()
    app.MainLoop()
