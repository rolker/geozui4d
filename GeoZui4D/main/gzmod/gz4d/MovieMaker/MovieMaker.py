#!/usr/bin/python
# gz
# movieMaker
# Controls the creation of movies

from MovieMakerUI import *
import sys
import pickle
import math
import time
import gz4d

class timeEvent:
    def __init__(self, id, time, name='name'):
        self.name = name
        self.time = float(time)
        self.id = id

class scriptEvent(timeEvent):
    def __init__(self, id, time, name='script event', script = ''):
        timeEvent.__init__(self, id , time,name)
        self.script = script
        self.type = 's'
        
class posEvent(timeEvent):
    def __init__(self, id, time, name='pos event'):
        timeEvent.__init__(self, id,time,name)
        self.type = 'p'
        self.startPosition = [0,0,0] # x,y,z
        self.startAttitude = [0,0,0] # yaw, pitch, roll
        self.startScale = 1.0
        #startTime -> time
        self.startTimeScale = 1.0
        
        self.endPosition = [0,0,0] # x,y,z
        self.endAttitude = [0,0,0] # yaw, pitch, roll
        self.endScale = 1.0
        self.endTime = 0.0
        self.endTimeScale = 1.0
        
        self.controlCamera = True
        self.controlTime = True

class scriptData:
    def __init__(self):
        self.events = {}
        self.selectedEvent = 0
        self.nextID = 1
        
        self.outFilename = "out.mpg"
        self.width = 720
        self.height = 480
        self.fps = 30
        self.vbr = 9800000
        self.abr = 192000
        self.watermark = True
        self.windows = {}
        self.spatialAA = "1.0"
        self.temporalAA = "1.0"
        
    def GetNextID(self):
        ret = self.nextID
        self.nextID += 1
        return ret

class frame:
    def __init__(self):
        self.videotime = 0.0
        
        self.time = 0.0
        self.timescale = 1.0
        
        self.position = [0.0,0.0,0.0]
        self.attitude = [0.0,0.0,0.0]
        self.scale = 1.0
        
        self.script = ""
        
        self.write = False
        self.weight = 1.0
        
        self.controlCamera = True
        self.controlTime = True
        
    def getGZCommand(self):
        if self.script != '':
            ret = ''.join((self.script,';'))
        else:
            ret = ''
        if self.controlCamera != False:
            ret = ' '.join((ret,'gz posable set Window.main xyzhprS',str(self.position[0]),str(self.position[1]),str(self.position[2]),str(self.attitude[0]),str(self.attitude[1]),str(self.attitude[2]),str(self.scale),';'))
        if self.controlTime != False:
            ret = ''.join((ret,' gztime set ',str(self.time),str(self.timescale)))
        return ret

class mmFrame(movieMakerFrame):
    def __init__(self, *args, **kwds):
        movieMakerFrame.__init__(self, *args, **kwds)
    
        self.gz = gz4d.Connection()
        self.gz.Connect()
        
        self.statusbar.SetStatusText('Connected to GeoZui4D')
        
        self.script = scriptData()
                
        self.eventList.InsertColumn(0,"Time",width=150)
        self.eventList.InsertColumn(1,"Type",width=40)
        self.eventList.InsertColumn(2,"Event",width=500)
        self.setFilename('')

        #self.codecs = {}
        #c =  self.sendCommand('getCodecs')[1]
        #for line in c.split('\n'):
            #details = line.split()
            #if len(details) == 3:
                #if details[2] == '(v)':
                    #self.codecs[details[0]] = details[1]
        #clist = self.codecs.keys()
        #clist.sort()
        #for c in clist:
            #n = self.codecCombo.Append(c,self.codecs[c])
            #if c == 'msmpeg4v2':
                #self.codecCombo.Select(n)

        self.additionalWindowsListBox.SetSelection(0,False)
        windows = self.gz.SendCommand('gz address list .Posable.Window')[1]
        for win in windows.split():
            w = win.rsplit('.',1)[1]
            if w != 'main':
                self.additionalWindowsListBox.Append(w)

    def sendCommand(self, cmd):
        s =  "global mainWindow; $mainWindow videoRecorder " + cmd
        return self.gz.SendCommand(s)

        
    def findPOSevent(self,time):
        i = 0
        ret = None
        while i < self.eventList.GetItemCount():
            e = self.script.events[self.eventList.GetItemData(i)]
            if e.type == 'p':
                if e.time <= time and e.endTime > time:
                    ret = e
            i += 1
        return ret
                
    def findScript(self,ftime,ttime):
        i = 0
        ret = ""
        while i < self.eventList.GetItemCount():
            e = self.script.events[self.eventList.GetItemData(i)]
            if e.type == 's':
                #print " e " + str(e.time) + " f " + str(ftime) + " t " + str(ttime)
                if e.time > ftime and e.time <= ttime:
                    #print "Bingo!"
                    ret = e.script
            i += 1
        return ret
                
    def setFilename(self,fname):
        self.filename = fname
        if fname:
            self.SetTitle(' - '.join(('Movie Maker',fname)))
        else:
            self.SetTitle('Movie Maker')
            
    def createFrameList(self,startTime = None):
        ret = []
        cPosition = [0.0,0.0,0.0]
        cAttitude = [0.0,0.0,0.0]
        cScale = 1.0;
        cTime = 0.0;
        cTimeScale = 1.0;
        cVideotime = 0.0
        
        ftime = 1.0/float(self.script.fps)
        
        controlCamera = True
        controlTime = True
        haveStart = 0
        i = 0
        
        if not startTime:
            startTime = self.script.events[self.eventList.GetItemData(0)].time
        endTime = startTime

        while i < self.eventList.GetItemCount():
            e = self.script.events[self.eventList.GetItemData(i)]
            if e.type == 'p' and haveStart != 1 and e.time >= startTime:
                cPosition = e.startPosition[:]
                cAttitude = e.startAttitude[:]
                cScale = e.startScale
                cTime = e.time
                cTimeScale = e.startTimeScale
                controlCamera = e.controlCamera
                controlTime = e.controlTime
                haveStart = 1
            if e.type == 'p':
                if endTime < e.endTime:
                    endTime = e.endTime
            else:
                if endTime < e.time:
                    endTime = e.time
            i += 1
        
        if haveStart == 0:
            return ret
        
        cTime = startTime
        while cTime < endTime:
            temporalAA = self.temporalAASpin.GetValue()
            for t in range(temporalAA):
                aaTime = ftime/float(5*temporalAA)
                f = frame()
                if t == 0:
                    tstep = ftime - (temporalAA-1)*aaTime
                    f.write = True
                else:
                    tstep = aaTime
                    f.write = False
                f.weight = 1/float(temporalAA)

                f.controlCamera = controlCamera
                f.controlTime = controlTime
                f.time = cTime
                f.timescale = cTimeScale
                f.position = cPosition[:]
                f.attitude = cAttitude[:]
                f.scale = cScale
                f.script = self.findScript(cTime - (cTimeScale*ftime), cTime)
                #print "f.script: " + f.script
                f.videotime = cVideotime
                ret.append(f)
                
                cTime += cTimeScale * tstep
                p = self.findPOSevent(cTime)
                if p != None:
                    dx = p.endPosition[0] - p.startPosition[0]
                    dy = p.endPosition[1] - p.startPosition[1]
                    dz = p.endPosition[2] - p.startPosition[2]
                    dts = p.endTimeScale - p.startTimeScale
                    
                    x = pow((cTime-p.time)/(p.endTime-p.time),p.startTimeScale/p.endTimeScale)
                    
                    cTimeScale = p.startTimeScale + x*dts
                    cPosition[0] = p.startPosition[0] + x*dx
                    cPosition[1] = p.startPosition[1] + x*dy
                    cPosition[2] = p.startPosition[2] + x*dz
                    dh = self.deltaAngle(p.startAttitude[0],p.endAttitude[0])
                    dp = self.deltaAngle(p.startAttitude[1],p.endAttitude[1])
                    dr = self.deltaAngle(p.startAttitude[2],p.endAttitude[2])
                    cAttitude[0] = p.startAttitude[0] + x*dh
                    cAttitude[1] = p.startAttitude[1] + x*dp
                    cAttitude[2] = p.startAttitude[2] + x*dr
                    dsrs = math.sqrt(p.endScale)-math.sqrt(p.startScale)
                    cScale = pow(math.sqrt(p.startScale)+x*dsrs,2)
                cVideotime += tstep
        return ret
    
    def deltaAngle(self,a,b):
        while a < 0.0:
            a += 360.0
        while b < 0.0:
            b += 360.0
        a = math.fmod(a,360.0)
        b = math.fmod(b,360.0)
        if b>a:
            if b-180.0 > a:
                b -= 360.0
        else:
            if a-180.0 > b:
                a -= 360
        return b-a

    def preview(self,startTime = None):
        self.sendCommand('set preview on')
        self.sendCommand('set width '+self.widthCombo.GetValue())
        self.sendCommand('set height '+self.heightCombo.GetValue())
        self.sendCommand('set useWatermark '+str(self.watermarkCheckbox.GetValue()))
        self.sendCommand('start')

        frames = self.createFrameList(startTime)
        i = 0;
        c = len(frames)
        st = time.time()
        prog = wx.ProgressDialog('Preview','Previewing movie',c,self,wx.PD_AUTO_HIDE|wx.PD_APP_MODAL|wx.PD_CAN_ABORT|wx.PD_ELAPSED_TIME|wx.PD_ESTIMATED_TIME|wx.PD_REMAINING_TIME)
        for f in frames:
            if(f.script != "" or time.time()-st <= f.videotime):
                self.gz.SendCommand(f.getGZCommand())
                self.statusbar.SetStatusText("Frame: "+ str(i) + " of "  + str(c))
            i += 1
            if not prog.Update(i)[0]:
                break
        
        prog.Destroy()
        self.sendCommand('stop')

    ###
    ### menu callbacks
    ###

    def OnFile_Exit(self,evt):
        self.Close()
        
    def OnFile_Open(self,evt):
        d = wx.FileDialog(self,defaultDir = self.gz.datadir, wildcard = "GZ Script files (*.gzscript)|*.gzscript",style=wx.OPEN)
        ret = d.ShowModal()
        if ret == wx.ID_OK:
            f = open(d.GetPath(),'r')
            self.setFilename(d.GetFilename())
            self.gz.datadir = d.GetDirectory()
            self.script = pickle.load(f)
            self.eventList.DeleteAllItems()
            for i in self.script.events:
                self.AddEventToList(self.script.events[i])
            self.outfilenameText.SetValue(self.script.outFilename)
            self.widthCombo.SetValue(str(self.script.width))
            self.heightCombo.SetValue(str(self.script.height))
            self.fpsCombo.SetValue(str(self.script.fps))
            self.vbrCombo.SetValue(str(self.script.vbr))
            self.abrCombo.SetValue(str(self.script.abr))
            try:
                self.watermarkCheckbox.SetValue(self.script.watermark)
                self.additionalWindowsListBox.DeselectAll()
                for i in self.script.windows:
                    self.additionalWindowsListBox.Select(i)
                self.spatialAATextCtrl.SetValue(self.script.spatialAA)
                self.temporalAASpin.SetValue(float(self.script.temporalAA))
            except AttributeError:
                pass
            
    def OnFile_Save(self,evt):
        d = wx.FileDialog(self,defaultDir = self.gz.datadir, wildcard = "GZ Script files (*.gzscript)|*.gzscript",style=wx.SAVE)
        if self.filename:
            d.SetFilename(self.filename)
        ret = d.ShowModal()
        if ret == wx.ID_OK:
            f = open(d.GetPath(),'w')
            self.setFilename(d.GetPath())
            #Event handling didn't take care of the combo boxes, so get it right now
            self.script.width = int(self.widthCombo.GetValue())
            self.script.height = int(self.heightCombo.GetValue())
            self.script.fps = int(self.fpsCombo.GetValue())
            self.script.vbr = int(self.vbrCombo.GetValue())
            self.script.abr = int(self.abrCombo.GetValue())
            pickle.dump(self.script,f)
            self.statusbar.SetStatusText(' '.join(('Saved',self.filename)))
        

    ###
    ###  global details callbacks
    ###

    def OnSelectOutfile(self,evt):
        fdlg = wx.FileDialog(self,style=wx.SAVE)
        fdlg.SetDirectory(self.gz.datadir)
        ret = fdlg.ShowModal()
        if ret == wx.ID_OK:
            self.outfilenameText.SetValue(fdlg.GetPath())
            
    def OnPreview(self,evt):
        self.preview()
        
    def OnPreviewFrom(self,evt):
        self.preview(self.script.events[self.script.selectedEvent].time)
    
    def OnRecord(self,evt):
        frames = self.createFrameList()

        self.sendCommand('set preview off')
        self.sendCommand('set filename '+self.outfilenameText.GetValue())
        self.sendCommand('set width '+self.widthCombo.GetValue())
        self.sendCommand('set height '+self.heightCombo.GetValue())
        self.sendCommand('set fps '+self.fpsCombo.GetValue())
        self.sendCommand('set videoBitrate '+self.vbrCombo.GetValue())
        self.sendCommand('set audioBitrate '+self.abrCombo.GetValue())
        self.sendCommand('set startTime '+str(frames[0].time))
        self.sendCommand('set spatialAA '+self.spatialAATextCtrl.GetValue())
        self.sendCommand('set useWatermark '+str(self.watermarkCheckbox.GetValue()))
        try:
            if self.codecCombo.GetSelection() != -1:
                self.sendCommand('set codec '+self.codecCombo.GetClientData(self.codecCombo.GetSelection()))
        except TypeError:
            pass

        aws = self.additionalWindowsListBox.GetSelections()
        if len(aws) > 0:
            withStr = ' with '
            for i in aws:
                withStr += self.additionalWindowsListBox.GetString(i)
        else:
            withStr = ''

        if not self.sendCommand('start'+withStr)[0]:
            print 'Error starting video recorder'
            return

        i = 0;
        c = len(frames)
        prog = wx.ProgressDialog('Recording','Recording movie',c,self,wx.PD_AUTO_HIDE|wx.PD_APP_MODAL|wx.PD_CAN_ABORT|wx.PD_ELAPSED_TIME|wx.PD_ESTIMATED_TIME|wx.PD_REMAINING_TIME)
        for f in frames:
            self.gz.SendCommand(f.getGZCommand())
            self.statusbar.SetStatusText("Frame: "+ str(i) + " of "  + str(c))
            self.sendCommand('addFrame '+str(f.weight))
            if f.write:
                self.sendCommand('recordFrame')
            i += 1
            if not prog.Update(i)[0]:
                break
        prog.Destroy()
        self.sendCommand('stop')
        
    def OnOutFilenameModified(self,evt):
        self.script.outFilename = self.outfilenameText.GetValue()
        
    def OnFPSModified(self,evt):
        try:
            self.script.fps = int(self.fpsCombo.GetValue())
        except ValueError:
            pass
        
    def OnWidthModified(self,evt):
        try:
            self.script.width = int(self.widthCombo.GetValue())
        except ValueError:
            pass
        
    def OnHeightModified(self,evt):
        try:
            self.script.height = int(self.heightCombo.GetValue())
        except ValueError:
            pass
        
    def OnVBRModified(self,evt):
        try:
            self.script.vbr = int(self.vbrCombo.GetValue())
        except ValueError:
            pass
        
    def OnABRModified(self,evt):
        try:
            self.script.abr = int(self.abrCombo.GetValue())
        except ValueError:
            pass
            
    def OnWindowsModified(self,evt):
        try:
            self.script.windows = self.additionalWindowsListBox.GetSelections()
        except ValueError:
            pass

    def OnWatermarkModified(self,evt):
        try:
            self.script.watermark = self.watermarkCheckbox.GetValue()
        except ValueError:
            pass
        
    def OnSpatialAAModified(self,evt):
        try:
            self.script.spatialAA = self.spatialAATextCtrl.GetValue()
        except ValueError:
            pass

    def OnTemporalAAModified(self,evt):
        try:
            self.script.temporalAA = str(self.temporalAASpin.GetValue())
        except ValueError:
            pass

    ###
    ###  adding and selecting events callbacks
    ###
        
    def AddEvent(self,e):
        self.script.events[e.id] = e
        self.AddEventToList(e)
        
    def AddEventToList(self,e):
        item = wx.ListItem()
        item.SetText(str(e.time))
        item.SetData(e.id)
        self.eventList.InsertItem(item)
        self.eventList.SetStringItem(item.GetId(),1,e.type)
        self.eventList.SetStringItem(item.GetId(),2,e.name)
        self.eventList.SortItems(lambda x,y :cmp(self.script.events[x].time,self.script.events[y].time))

    def OnAddScript(self,evt):
        se = scriptEvent(self.script.GetNextID(),self.gz.GetTime())
        self.AddEvent(se) 
        
    def OnAddPOS(self,evt):
        sp = posEvent(self.script.GetNextID(),self.gz.GetTime())
        self.AddEvent(sp)
        
    def OnEventSelected(self,evt):
        self.script.selectedEvent = evt.GetData()
        e = self.script.events[self.script.selectedEvent]
        self.eventTimeText.SetValue(str(e.time))
        self.eventNameText.SetValue(e.name)
        if e.type == 's': #script
            self.scriptDetailsScriptText.SetValue(e.script)
            self.scriptDetailsResultText.SetValue("")
            self.posDetailsPanel.Hide()
            self.scriptDetailsPanel.Show()
        else:
            self.StartXText.SetValue(str(e.startPosition[0]))
            self.StartYText.SetValue(str(e.startPosition[1]))
            self.StartZText.SetValue(str(e.startPosition[2]))
            
            self.StartHeadingText.SetValue(str(e.startAttitude[0]))
            self.StartPitchText.SetValue(str(e.startAttitude[1]))
            self.StartRollText.SetValue(str(e.startAttitude[2]))
            
            self.StartScaleText.SetValue(str(e.startScale))
            self.StartTimeText.SetValue(str(e.time))
            self.StartTimeScaleText.SetValue(str(e.startTimeScale))
            
            self.EndXText.SetValue(str(e.endPosition[0]))
            self.EndYText.SetValue(str(e.endPosition[1]))
            self.EndZText.SetValue(str(e.endPosition[2]))
            
            self.EndHeadingText.SetValue(str(e.endAttitude[0]))
            self.EndPitchText.SetValue(str(e.endAttitude[1]))
            self.EndRollText.SetValue(str(e.endAttitude[2]))
            
            self.EndScaleText.SetValue(str(e.endScale))
            self.EndTimeText.SetValue(str(e.endTime))
            self.EndTimeScaleText.SetValue(str(e.endTimeScale))
            
            self.controlCameraCheckbox.SetValue(e.controlCamera)
            self.controlTimeCheckbox.SetValue(e.controlTime)
            
            self.scriptDetailsPanel.Hide()
            self.posDetailsPanel.Show()
        self.blankDetailsPanel.Layout()
        
    ###
    ###  event common callbacks (name and time)
    ###
    
    def OnTimeTextModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.eventTimeText.GetValue())
        except ValueError:
            v = 0.0
        if e.time != v:
            e.time = v
            i = self.eventList.FindItemData(-1,e.id)
            self.eventList.SetItemText(i,str(e.time))
            self.eventList.SortItems(lambda x,y :cmp(self.script.events[x].time,self.script.events[y].time))
            if e.type == 'p':
                self.StartTimeText.SetValue(str(v))
            
    def OnGetGZTime(self,evt):
        self.eventTimeText.SetValue(str(self.gz.GetTime()))
        
    def OnNameTextModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        if e.name != self.eventNameText.GetValue():
            e.name = self.eventNameText.GetValue()
            i = self.eventList.FindItemData(-1,e.id)
            item = self.eventList.GetItem(i,2)
            item.SetText(e.name)
            self.eventList.SetItem(item)
    
    ###
    ###  script event callbacks
    ###
    
    def OnScriptTextModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        if e.script != self.scriptDetailsScriptText.GetValue():
            e.script = self.scriptDetailsScriptText.GetValue()

    def OnExecuteScript(self,evt):
        s = self.scriptDetailsScriptText.GetValue()
        if s == "":
            self.scriptDetailsResultText.SetDefaultStyle(wx.TextAttr(wx.RED))
            self.scriptDetailsResultText.AppendText("Execute what?!?\n")
        else:
            r = self.gz.SendCommand(s)
            if r[0] == 1:
                self.scriptDetailsResultText.SetDefaultStyle(wx.TextAttr(wx.RED))
            else:
                self.scriptDetailsResultText.SetDefaultStyle(wx.TextAttr(wx.BLUE))
            self.scriptDetailsResultText.AppendText(r[1]+"\n")
            
    ###
    ###  pos event callbacks (position, orientation and scale of viewpoint and time)
    ###
    
    def OnGotoStart(self,evt):
        e = self.script.events[self.script.selectedEvent]
        ret = ""
        if e.controlCamera != False:
            ret = "gz posable set Window.main xyzhprS "
            ret += str(e.startPosition[0]) + " "
            ret += str(e.startPosition[1]) + " "
            ret += str(e.startPosition[2]) + " "
            ret += str(e.startAttitude[0]) + " "
            ret += str(e.startAttitude[1]) + " "
            ret += str(e.startAttitude[2]) + " "
            ret += str(e.startScale) + ";"
        if e.controlTime != False:
            ret += " gztime set " + " " + str(e.time) + " " + str(e.startTimeScale)
        print ret
        self.gz.SendCommand(ret)
    
    def OnGotoEnd(self,evt):
        e = self.script.events[self.script.selectedEvent]
        ret = ""
        if e.controlCamera != False:
          ret = "gz posable set Window.main xyzhprS "
          ret += str(e.endPosition[0]) + " "
          ret += str(e.endPosition[1]) + " "
          ret += str(e.endPosition[2]) + " "
          ret += str(e.endAttitude[0]) + " "
          ret += str(e.endAttitude[1]) + " "
          ret += str(e.endAttitude[2]) + " "
          ret += str(e.endScale) + ";"
        if e.controlTime != False:
          ret += " gztime set " + " " + str(e.endTime) + " " + str(e.endTimeScale)
        self.gz.SendCommand(ret)
        
    ### Start position
    
    def OnControlCameraModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            b = self.controlCameraCheckbox.GetValue()
        except ValueError:
            b = True
        if e.controlCamera != b:
            e.controlCamera = b

    def OnStartXModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartXText.GetValue())
        except ValueError:
            v = 0.0
        if e.startPosition[0] != v:
            e.startPosition[0] = v

    def OnStartYModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartYText.GetValue())
        except ValueError:
            v = 0.0
        if e.startPosition[1] != v:
            e.startPosition[1] = v

    def OnStartZModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartZText.GetValue())
        except ValueError:
            v = 0.0
        if e.startPosition[2] != v:
            e.startPosition[2] = v
            
    def OnGetStartPosGZ(self,evt):
        p = self.gz.GetPosition()
        self.StartXText.SetValue(str(p[0]))
        self.StartYText.SetValue(str(p[1]))
        self.StartZText.SetValue(str(p[2]))
    
    ### Start attitude
    
    def OnStartHeadingModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartHeadingText.GetValue())
        except ValueError:
            v = 0.0
        if e.startAttitude[0] != v:
            e.startAttitude[0] = v
    
    def OnStartPitchModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartPitchText.GetValue())
        except ValueError:
            v = 0.0
        if e.startAttitude[1] != v:
            e.startAttitude[1] = v
    
    def OnStartRollModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartRollText.GetValue())
        except ValueError:
            v = 0.0
        if e.startAttitude[2] != v:
            e.startAttitude[2] = v
            
    def OnGetStartAttitudeGZ(self,evt):
        a = self.gz.GetAttitude()
        self.StartHeadingText.SetValue(str(a[0]))
        self.StartPitchText.SetValue(str(a[1]))
        self.StartRollText.SetValue(str(a[2]))
        
    ### Start scale
        
    def OnStartScaleModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartScaleText.GetValue())
        except ValueError:
            v = 0.0
        if e.startScale != v:
            e.startScale = v

    def OnGetStartScaleGZ(self,evt):
        self.StartScaleText.SetValue(str(self.gz.GetScale()))
        
    ### Start time

    def OnControlTimeModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            b = self.controlTimeCheckbox.GetValue()
        except ValueError:
            b = True
        if e.controlTime != b:
            e.controlTime = b
            
    def OnStartTimeScaleModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.StartTimeScaleText.GetValue())
        except ValueError:
            v = 0.0
        if e.startTimeScale != v:
            e.startTimeScale = v

    def OnGetStartTimeScaleGZ(self,evt):
        self.StartTimeScaleText.SetValue(str(self.gz.GetTimeScale()))

    def OnGetAllStartGZ(self,evt):
        self.OnGetStartPosGZ(evt)
        self.OnGetStartAttitudeGZ(evt)
        self.OnGetStartScaleGZ(evt)
        self.OnGetGZTime(evt)
        self.OnGetStartTimeScaleGZ(evt)


    ### End position

    def OnEndXModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndXText.GetValue())
        except ValueError:
            v = 0.0
        if e.endPosition[0] != v:
            e.endPosition[0] = v

    def OnEndYModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndYText.GetValue())
        except ValueError:
            v = 0.0
        if e.endPosition[1] != v:
            e.endPosition[1] = v

    def OnEndZModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndZText.GetValue())
        except ValueError:
            v = 0.0
        if e.endPosition[2] != v:
            e.endPosition[2] = v

    def OnGetEndPosGZ(self,evt):
        p = self.gz.GetPosition()
        self.EndXText.SetValue(str(p[0]))
        self.EndYText.SetValue(str(p[1]))
        self.EndZText.SetValue(str(p[2]))
    
    ### End attitude
    
    def OnEndHeadingModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndHeadingText.GetValue())
        except ValueError:
            v = 0.0
        if e.endAttitude[0] != v:
            e.endAttitude[0] = v
    
    def OnEndPitchModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndPitchText.GetValue())
        except ValueError:
            v = 0.0
        if e.endAttitude[1] != v:
            e.endAttitude[1] = v
    
    def OnEndRollModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndRollText.GetValue())
        except ValueError:
            v = 0.0
        if e.endAttitude[2] != v:
            e.endAttitude[2] = v
            
    def OnGetEndAttitudeGZ(self,evt):
        a = self.gz.GetAttitude()
        self.EndHeadingText.SetValue(str(a[0]))
        self.EndPitchText.SetValue(str(a[1]))
        self.EndRollText.SetValue(str(a[2]))
        
    def OnEndScaleModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndScaleText.GetValue())
        except ValueError:
            v = 0.0
        if e.endScale != v:
            e.endScale = v

    def OnGetEndScaleGZ(self,evt):
        self.EndScaleText.SetValue(str(self.gz.GetScale()))
        
    ### end time
    
    def OnEndTimeModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndTimeText.GetValue())
        except ValueError:
            v = 0.0
        if e.endTime != v:
            e.endTime = v

    def OnGetEndTimeGZ(self,evt):
        self.EndTimeText.SetValue(str(self.gz.GetTime()))

    def OnEndTimeScaleModified(self,evt):
        e = self.script.events[self.script.selectedEvent]
        try:
            v = float(self.EndTimeScaleText.GetValue())
        except ValueError:
            v = 0.0
        if e.endTimeScale != v:
            e.endTimeScale = v

    def OnGetEndTimeScaleGZ(self,evt):
        self.EndTimeScaleText.SetValue(str(self.gz.GetTimeScale()))

    def OnGetAllEndGZ(self,evt):
        self.OnGetEndPosGZ(evt)
        self.OnGetEndAttitudeGZ(evt)
        self.OnGetEndScaleGZ(evt)
        self.OnGetEndTimeGZ(evt)
        self.OnGetEndTimeScaleGZ(evt)


class MovieMaker(wx.App):

    def OnInit(self):
        f = mmFrame(None)
        f.Show()
        self.SetTopWindow(f)
        return True
