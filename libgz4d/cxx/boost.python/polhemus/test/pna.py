#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import math

from OpenGL.GL import *
from OpenGL.GLU import *
from PyQt4 import QtCore, QtGui
from PyQt4.QtOpenGL import *
import gz4d
import gz4d.qt

import threading

class PNAContentDisplay(gz4d.qt.GLWidget):
    '''
    Widget for displaying Personal Navigation Assistant content.
    '''
    
    def __init__(self, parent, ft):
        gz4d.qt.GLWidget.__init__(self, parent)
        self.setMinimumSize(500, 500)
        self.fastrak = ft

        self.scene = gz4d.s57.S57("/home/roland/work/ENC_ROOT/NH_ENCProdCat.xml")
        #self.scene = gz4d.x3d.X3D()
        #self.scene.Load('/home/roland/work/models/ships/NancyFoster/NancyFoster.x3d')

        self.app = gz4d.Application()
        self.app.AddCanvas(self.canvas)
        self.canvas.SetPixelSize(0.000111765,0.000117188) #xenarc display at 1360x768

        self.app.CreateARViewer(self.scene)

        self.pe = gz4d.polhemus.PolhemusEvents(ft)

        #self.canvas.AppendHandler(self.pe)
        #self.scene = gz4d.s57.Viewer("/home/roland/work/ENC_ROOT/NH_ENCProdCat.xml")

    def timerEvent(self, event):
        self.propagate(self.pe())
        gz4d.qt.GLWidget.timerEvent(self,event)

class PNADebugDisplay(QGLWidget):
    '''
    Widget for display of Personal navigation Assistant.
    '''
    
    def __init__(self, parent, ft):
        QGLWidget.__init__(self, parent)
        self.setMinimumSize(500, 500)
        self.fastrak = ft

    def drawSensor(self):
        glBegin(GL_TRIANGLES)

        glColor3f(1.0,0.0,0.0)
        glVertex3f(0.0, 0.01, 0.0)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(0.01, 0.0, 0.0)

        glColor3f(0.0,1.0,0.0)
        glVertex3f(0.0, 0.0, 0.01)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(0.01, 0.0, 0.0)

        glColor3f(0.0,0.0,1.0)
        glVertex3f(0.0, 0.0, 0.01)
        glVertex3f(0.0, 0.0, 0.0)
        glVertex3f(0.0, 0.01, 0.0)

        glEnd()

    def drawScreen(self):
        glBegin(GL_QUADS)

        glColor3f(0.9,0.9,0.9)
        glVertex3f(-0.097, 0.06, -0.001)
        glVertex3f(-0.097, -0.06, -0.001)
        glVertex3f(0.097, -0.06, -0.001)
        glVertex3f(0.097, 0.06, -0.001)

        glColor3f(0.2,0.2,0.2)
        glVertex3f(-0.078, 0.047, 0.0)
        glVertex3f(-0.078, -0.047, 0.0)
        glVertex3f(0.078, -0.047, 0.0)
        glVertex3f(0.078, 0.047, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(-0.063, -0.056, 0.0)
        glVertex3f(-0.063, -0.059, 0.0)
        glVertex3f(-0.053, -0.059, 0.0)
        glVertex3f(-0.053, -0.056, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(-0.05, -0.056, 0.0)
        glVertex3f(-0.05, -0.059, 0.0)
        glVertex3f(-0.04, -0.059, 0.0)
        glVertex3f(-0.04, -0.056, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(-0.037, -0.056, 0.0)
        glVertex3f(-0.037, -0.059, 0.0)
        glVertex3f(-0.027, -0.059, 0.0)
        glVertex3f(-0.027, -0.056, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(-0.024, -0.056, 0.0)
        glVertex3f(-0.024, -0.059, 0.0)
        glVertex3f(-0.014, -0.059, 0.0)
        glVertex3f(-0.014, -0.056, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(0.021, -0.056, 0.0)
        glVertex3f(0.021, -0.059, 0.0)
        glVertex3f(0.035, -0.059, 0.0)
        glVertex3f(0.035, -0.056, 0.0)

        glColor3f(0.0,0.0,0.0)
        glVertex3f(0.053, -0.056, 0.0)
        glVertex3f(0.053, -0.059, 0.0)
        glVertex3f(0.076, -0.059, 0.0)
        glVertex3f(0.076, -0.056, 0.0)

        glEnd()


    def paintGL(self):
        '''
        Drawing routine
        '''

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glPushMatrix()

        p = self.fastrak.state[0]['position']
        if p is not None:
            x = (p[0]+15.0)/100.0
            y = (p[1]-18.0)/100.0
            z = p[2]/100.0
            glTranslatef(x,y,z)
        o = self.fastrak.state[0]['orientation']
        if o is not None:
            gz4d.glo.MultMatrix(gz4d.Rotationf(gz4d.Boost.quaternionf(o[0],o[1],o[2],o[3])).GetMatrix())

        self.drawSensor()
        #gz4d.glo.MultMatrix(gz4d.Rotationf(math.radians(-90.0),gz4d.Pointf(0.0,0.0,1.0)).GetMatrix())
        #gz4d.glo.MultMatrix(gz4d.Rotationf(math.radians(-90.0),gz4d.Pointf(0.0,1.0,0.0)).GetMatrix())

        #glTranslatef(-.1,-.065,0.08)

        glTranslatef(0.0, 0.0, 0.025)

        self.drawScreen()

        glPopMatrix()


    def resizeGL(self, w, h):
        '''
        Resize the GL window 
        '''
        
        glViewport(0, 0, w, h)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(20.0, float(w)/float(h), 0.1, 4.0)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        gluLookAt(0.0, 0.0, 0.75, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0)

    
    def initializeGL(self):
        '''
        Initialize GL
        '''
        
        # set viewing projection
        glClearColor(0.0, 0.0, 0.0, 1.0)
        glDrawBuffer(GL_BACK)
        glEnable(GL_DEPTH_TEST)

class PNADisplay(QtGui.QWidget):
    def __init__(self, parent, ft):
        QtGui.QWidget.__init__(self, parent)
        self.layout = QtGui.QHBoxLayout()
        #self.debug_widget = PNADebugDisplay(self,ft)
        self.widget = PNAContentDisplay(self,ft)
        #self.layout.addWidget(self.debug_widget)
        self.layout.addWidget(self.widget)
        self.setLayout(self.layout)

        self.idle_timer = QtCore.QTimer()
        QtCore.QObject.connect(self.idle_timer, QtCore.SIGNAL("timeout()"), self.idle)
        self.idle_timer.start(1000/30)

    def idle(self):
        #self.debug_widget.updateGL()
        self.widget.updateGL()

    

class PNA(QtGui.QMainWindow):

    def __init__(self,ft):
        QtGui.QMainWindow.__init__(self)
        self.widget = PNADisplay(self,ft)
        self.setCentralWidget(self.widget)



if __name__ == '__main__':
    app = QtGui.QApplication(['Personal Navigation Assistant'])
    ft = gz4d.polhemus.Fastrak('/dev/ttyUSB0')
    window = PNA(ft)
    window.show()
    app.exec_()
    ft.Stop()
