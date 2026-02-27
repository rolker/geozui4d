#!/usr/bin/env python

import imuCalibratorUI
import gz4d
import wx
import wx.glcanvas
import numpy
from OpenGL.GL import *


class Quadratic:
    def __init__(self, a=0, b=1, c=0):
        self.a = a
        self.b = b
        self.c = c

    def __call__(self,x):
        return self.a*x*x+self.b*x+self.c

    def __str__(self):
        return 'a='+str(self.a)+', b='+str(self.b)+', c='+str(self.c)


    def Init(self, equations):
        '''
        equations is a list of 3 pairs (x,y) where y = ax^2 + bx + c
        '''
        mat = []
        for e in equations:
            mat.append([e[0]**2,e[0],1,e[1]])

        try:
            r1factor = 1/mat[0][0]
            mat[0][0] *= r1factor
            mat[0][1] *= r1factor
            mat[0][2] *= r1factor
            mat[0][3] *= r1factor
            r2factor = mat[1][0]
            mat[1][0] -= mat[0][0]*r2factor
            mat[1][1] -= mat[0][1]*r2factor
            mat[1][2] -= mat[0][2]*r2factor
            mat[1][3] -= mat[0][3]*r2factor
            r3factor = mat[2][0]
            mat[2][0] -= mat[0][0]*r3factor
            mat[2][1] -= mat[0][1]*r3factor
            mat[2][2] -= mat[0][2]*r3factor
            mat[2][3] -= mat[0][3]*r3factor
    
            r2factor = 1/mat[1][1]
            mat[1][1] *= r2factor
            mat[1][2] *= r2factor
            mat[1][3] *= r2factor
            r1factor = mat[0][1]
            mat[0][1] -= mat[1][1]*r1factor
            mat[0][2] -= mat[1][2]*r1factor
            mat[0][3] -= mat[1][3]*r1factor
            r3factor = mat[2][1]
            mat[2][1] -= mat[1][1]*r3factor
            mat[2][2] -= mat[1][2]*r3factor
            mat[2][3] -= mat[1][3]*r3factor
    
            r3factor = 1/mat[2][2]
            mat[2][2] *= r3factor
            mat[2][3] *= r3factor
    
            r1factor = mat[0][2]
            mat[0][2] -= mat[2][2]*r1factor
            mat[0][3] -= mat[2][3]*r1factor
    
            r2factor = mat[1][2]
            mat[1][2] -= mat[2][2]*r2factor
            mat[1][3] -= mat[2][3]*r2factor
    
            self.a = mat[0][3]
            self.b = mat[1][3]
            self.c = mat[2][3]
        except ZeroDivisionError:
            pass

class Calibration:
    def __init__(self):
        self.orientations = ('rfu','bld','luf','fdl','ubr','drb')
        self.omap = {'u':'up', 'd':'down', 'l':'left', 'r':'right', 'f':'forward', 'b':'back'}
        self.sensors = (('m','magnetometer'),('a','accelerometer'),('g','gyroscope'))
        self.axis = ('x','y','z')

        self.maps = {}
        for s,sl in self.sensors:
            self.maps[s] = {}
            for a in self.axis:
                self.maps[s][a] = Quadratic()

        self.data = None

        self.means = {}
        for o in self.orientations:
            self.means[o] = {}
            for s,sl in self.sensors:
                self.means[o][s] = {}
                for a in self.axis:
                    self.means[o][s][a] = 0.0

        self.bufferSize = 100
        self.collecting = None

    def __call__(self,state):
        ret = gz4d.IMU.State()
        ret.time = state.time
        ret.accel[0] = self.maps['a']['x'](state.accel[0])
        ret.accel[1] = self.maps['a']['y'](state.accel[1])
        ret.accel[2] = self.maps['a']['z'](state.accel[2])
        ret.mag[0] = self.maps['m']['x'](state.mag[0])
        ret.mag[1] = self.maps['m']['y'](state.mag[1])
        ret.mag[2] = self.maps['m']['z'](state.mag[2])
        ret.gyro[0] = self.maps['m']['x'](state.gyro[0])
        ret.gyro[1] = self.maps['g']['y'](state.gyro[1])
        ret.gyro[2] = self.maps['g']['z'](state.gyro[2])
        return ret

    def Calculate(self):
        for s, sl in self.sensors:
            for a in self.axis:
                i = {'x':0, 'y':1, 'z':2}[a]
                eqs = [[0.0,-1],[0.0,0],[0.0,1]]
                for o in self.orientations:
                    if o[i] == 'u':
                        eqs[0][0] = self.means[o][s][a]
                    else:
                        if o[i] == 'd':
                            eqs[2][0] = self.means[o][s][a]
                        else:
                            eqs[1][0] += self.means[o][s][a]*0.25

                self.maps[s][a].Init(eqs)

    def Start(self,o):
        self.data = {}
        for s,sl in self.sensors:
            self.data[s] = {}
            for a in self.axis:
                self.data[s][a] = []
        self.collecting = o

    def Add(self,s):
        if self.collecting is not None:
            self.data['m']['x'].append(s.mag[0])
            self.data['m']['y'].append(s.mag[1])
            self.data['m']['z'].append(s.mag[2])
            self.data['a']['x'].append(s.accel[0])
            self.data['a']['y'].append(s.accel[1])
            self.data['a']['z'].append(s.accel[2])
            self.data['g']['x'].append(s.gyro[0])
            self.data['g']['y'].append(s.gyro[1])
            self.data['g']['z'].append(s.gyro[2])

            if len(self.data['m']['x']) == self.bufferSize:
                for s,sl in self.sensors:
                    for a in self.axis:
                        self.means[self.collecting][s][a] = numpy.average(self.data[s][a])
                self.collecting = None
                self.data = None
                self.Calculate()

    def GetValueString(self,o,s,a):
        return str(self.means[o][s][a])

class GLFrame(wx.Frame):
    def __init__(self, *args, **kwds):
        kwds["style"] = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
        wx.Frame.__init__(self, *args, **kwds)

        self.GLinitialized = False
        attribList = (wx.glcanvas.WX_GL_RGBA, # RGBA
                      wx.glcanvas.WX_GL_DOUBLEBUFFER, # Double Buffered
                      wx.glcanvas.WX_GL_DEPTH_SIZE, 24) # 24 bit

        #
        # Create the canvas
        self.canvas = wx.glcanvas.GLCanvas(self, attribList=attribList)

        #
        # Set the event handlers.
        self.canvas.Bind(wx.EVT_ERASE_BACKGROUND, self.processEraseBackgroundEvent)
        self.canvas.Bind(wx.EVT_SIZE, self.processSizeEvent)
        self.canvas.Bind(wx.EVT_PAINT, self.processPaintEvent)

        self.data = []
        self.dataSize = 300
        self.dataIndex = 0


    def AddData(self,d):
        if len(self.data) < 300:
            self.data.append(d)
        else:
            self.data[self.dataIndex] = d
            self.dataIndex += 1
            if self.dataIndex == 300:
                self.dataIndex = 0

    #
    # Canvas Proxy Methods

    def GetGLExtents(self):
        """Get the extents of the OpenGL canvas."""
        return self.canvas.GetClientSize()

    def SwapBuffers(self):
        """Swap the OpenGL buffers."""
        self.canvas.SwapBuffers()

    #
    # wxPython Window Handlers

    def processEraseBackgroundEvent(self, event):
        """Process the erase background event."""
        pass # Do nothing, to avoid flashing on MSWin

    def processSizeEvent(self, event):
        """Process the resize event."""
        if self.canvas.GetContext():
            # Make sure the frame is shown before calling SetCurrent.
            self.Show()
            self.canvas.SetCurrent()

            size = self.GetGLExtents()
            self.OnReshape(size.width, size.height)
            self.canvas.Refresh(False)
        event.Skip()

    def processPaintEvent(self, event):
        """Process the drawing event."""
        self.canvas.SetCurrent()

        # This is a 'perfect' time to initialize OpenGL ... only if we need to
        if not self.GLinitialized:
            self.OnInitGL()
            self.GLinitialized = True

        self.OnDraw()
        event.Skip()

    #
    # GLFrame OpenGL Event Handlers

    def OnInitGL(self):
        """Initialize OpenGL for use in the window."""
        glClearColor(1, 1, 1, 1)

    def OnReshape(self, width, height):
        """Reshape the OpenGL viewport based on the dimensions of the window."""
        glViewport(0, 0, width, height)

        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def OnDraw(self, *args, **kwargs):
        "Draw the window."
        glClear(GL_COLOR_BUFFER_BIT)

        if len(self.data) == 0:
            self.SwapBuffers()
            return

        glColor(0,0,0)
        glBegin(GL_LINES)
        glVertex(.5,-.5)
        glVertex(.5+self.data[self.dataIndex-1].mag[0]/2.5,-.5+self.data[self.dataIndex-1].mag[1]/2.5)
        glEnd()

        glColor(1, 0, 0)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),.5+self.data[di].accel[0]/2.5)
        glEnd()

        glColor(0, 1, 0)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),.5+self.data[di].accel[1]/2.5)
        glEnd()

        glColor(0, 0, 1)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),.5+self.data[di].accel[2]/2.5)
        glEnd()

        glColor(1, 0, 0)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),-.5+self.data[di].mag[0]/2.5)
        glEnd()

        glColor(0, 1, 0)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),-.5+self.data[di].mag[1]/2.5)
        glEnd()

        glColor(0, 0, 1)
        glBegin(GL_LINE_STRIP)
        for i in range(len(self.data)):
            di = self.dataIndex + i
            if di >= self.dataSize:
                di -= self.dataSize
            glVertex(-1.0+i/float(self.dataSize),-.5+self.data[di].mag[2]/2.5)
        glEnd()


        self.SwapBuffers()


class CalibratorFrame(imuCalibratorUI.MainFrame):
    def __init__(self, *args, **kwds):
        imuCalibratorUI.MainFrame.__init__(self, *args, **kwds)


        self.controls = {}
        self.calibration = Calibration()

        dataGridSizer = wx.FlexGridSizer(1, 5, 5, 5)
        dataGridSizer.Add((20, 20))
        dataGridSizer.Add((20, 20))
        dataGridSizer.Add(wx.StaticText(self, -1,'X'), flag=wx.ALIGN_CENTER_HORIZONTAL)
        dataGridSizer.Add(wx.StaticText(self, -1,'Y'), flag=wx.ALIGN_CENTER_HORIZONTAL)
        dataGridSizer.Add(wx.StaticText(self, -1,'Z'), flag=wx.ALIGN_CENTER_HORIZONTAL)
        dataGridSizer.AddGrowableCol(2)
        dataGridSizer.AddGrowableCol(3)
        dataGridSizer.AddGrowableCol(4)

        for s,sl in self.calibration.sensors:
            self.controls[s] = {}
            for t in ('raw','scaled'):
                self.controls[s][t] = {}
                if t == 'raw':
                    dataGridSizer.Add(wx.StaticText(self, -1, sl))
                else:
                    dataGridSizer.Add((20,20))
                dataGridSizer.Add(wx.StaticText(self, -1, t))
                for a in self.calibration.axis:
                    self.controls[s][t][a] = wx.TextCtrl(self, -1, style=wx.TE_READONLY)
                    dataGridSizer.Add(self.controls[s][t][a],flag=wx.EXPAND)

        for o in self.calibration.orientations:
            self.controls[o] = {}
            label = ''
            for i in range(len(self.calibration.axis)):
                a = self.calibration.axis[i]
                label += a + ': ' + self.calibration.omap[o[i]] + ' '

            for s,sl in self.calibration.sensors:
                if s == 'm':
                    dataGridSizer.Add(wx.StaticText(self, -1,label))
                if s == 'a':
                    b = wx.Button(self,label='Get Data',name=o)
                    dataGridSizer.Add(b)
                    self.Bind(wx.EVT_BUTTON,self.OnGetData,b)
                if s == 'g':
                    dataGridSizer.Add((20,20))
                dataGridSizer.Add(wx.StaticText(self, -1,s), flag=wx.ALIGN_CENTER_HORIZONTAL)
                self.controls[o][s] = {}
                for a in self.calibration.axis:
                    self.controls[o][s][a] = wx.TextCtrl(self, -1, style=wx.TE_READONLY)
                    dataGridSizer.Add(self.controls[o][s][a],flag=wx.EXPAND)

        self.GetSizer().Add(dataGridSizer, proportion=1, flag=wx.EXPAND)
        self.GetSizer().Fit(self)
        self.Layout()


        self.imu = None
        self.queue = None
        self.filter = None

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER,self.OnIdle)

        self.gl = GLFrame(self)
        self.gl.Show()

    def OnConnectButton(self, event):
        if self.imu is None:
            #connecting
            self.filter = gz4d.IMU.Filter()
            self.imu = gz4d.IMU.BTSource()
            self.queue = self.imu.NewOutQueue()
            self.imu.Open(self.btAddressTextCtrl.GetValue().encode('ascii'))
            self.connectButton.SetLabel('Disconnect')
            self.timer.Start(10)
        else:
            self.timer.Stop()
            self.imu = None
            self.connectButton.SetLabel('Connect')

    def OnIdle(self, event):
        if self.queue is not None:
            while not self.queue.empty():
                s = self.queue.pop()
                fs = self.filter(s)
                self.calibration.Add(fs)

                self.controls['m']['raw']['x'].SetValue(str(fs.mag[0]))
                self.controls['m']['raw']['y'].SetValue(str(fs.mag[1]))
                self.controls['m']['raw']['z'].SetValue(str(fs.mag[2]))
                self.controls['a']['raw']['x'].SetValue(str(fs.accel[0]))
                self.controls['a']['raw']['y'].SetValue(str(fs.accel[1]))
                self.controls['a']['raw']['z'].SetValue(str(fs.accel[2]))
                self.controls['g']['raw']['x'].SetValue(str(fs.gyro[0]))
                self.controls['g']['raw']['y'].SetValue(str(fs.gyro[1]))
                self.controls['g']['raw']['z'].SetValue(str(fs.gyro[2]))

                cfs = self.calibration(fs)
                self.gl.AddData(cfs)

                self.controls['m']['scaled']['x'].SetValue(str(cfs.mag[0]))
                self.controls['m']['scaled']['y'].SetValue(str(cfs.mag[1]))
                self.controls['m']['scaled']['z'].SetValue(str(cfs.mag[2]))
                self.controls['a']['scaled']['x'].SetValue(str(cfs.accel[0]))
                self.controls['a']['scaled']['y'].SetValue(str(cfs.accel[1]))
                self.controls['a']['scaled']['z'].SetValue(str(cfs.accel[2]))
                self.controls['g']['scaled']['x'].SetValue(str(cfs.gyro[0]))
                self.controls['g']['scaled']['y'].SetValue(str(cfs.gyro[1]))
                self.controls['g']['scaled']['z'].SetValue(str(cfs.gyro[2]))

                for o in self.calibration.orientations:
                    for s,sl in self.calibration.sensors:
                        for a in self.calibration.axis:
                            self.controls[o][s][a].SetValue(self.calibration.GetValueString(o,s,a))


            self.gl.Refresh()

            if self.imu is None:
                self.queue = None
                self.filter = None

    def OnGetData(self, event):
        self.calibration.Start(event.GetEventObject().GetName())


if __name__ == "__main__":
    app = wx.PySimpleApp(0)
    wx.InitAllImageHandlers()
    mainFrame = CalibratorFrame(None, -1, "")
    app.SetTopWindow(mainFrame)
    mainFrame.Show()
    app.MainLoop()
