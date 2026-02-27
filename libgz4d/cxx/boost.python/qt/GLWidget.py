#!/usr/bin/env python
# -*- coding: utf-8 -*-

import gz4d.qt
from PyQt4 import QtCore
from PyQt4.QtOpenGL import *

class GLWidget(QGLWidget):
    buttonMap = {QtCore.Qt.NoButton:gz4d.ui.Button.NoButton, QtCore.Qt.LeftButton:gz4d.ui.Button.LeftButton}
    def __init__(self, parent, canvas = None):
        QGLWidget.__init__(self, parent)
        if canvas is None:
            self.canvas = gz4d.qt.Canvas(self)
        else:
            self.canvas = canvas
        self.startTimer(0)

    def resizeGL(self, w, h):
        self.canvas.OnReshape(w,h)

    def paintGL(self):
        self.canvas.OnDisplay()

    def mousePressEvent(self, event):
        self.mouseMoveEvent(event)
        pb =  gz4d.ui.PointerButton()
        pb.button = GLWidget.buttonMap[event.button()]
        pb.direction = gz4d.ui.Direction.Down
        self.propagate(pb)

    def mouseReleaseEvent(self, event):
        self.mouseMoveEvent(event)
        pb =  gz4d.ui.PointerButton()
        pb.button = GLWidget.buttonMap[event.button()]
        pb.direction = gz4d.ui.Direction.Up
        self.propagate(pb)

    def mouseMoveEvent(self, event):
        m = gz4d.ui.PointerPosition()
        m.position[0] = event.x()
        m.position[1] = self.canvas.flipY(event.y())
        self.propagate(m)

    def wheelEvent(self, event):
        self.mouseMoveEvent(event)
        pb =  gz4d.ui.PointerButton()
        pb.button = gz4d.ui.Button.Wheel
        if(event.delta()>0):
            pb.direction = gz4d.ui.Direction.Forward
        else:
            pb.direction = gz4d.ui.Direction.Back
        self.propagate(pb)

    def timerEvent(self, event):
        i = gz4d.ui.Idle()
        self.propagate(i)

    def propagate(self,es):
        e = gz4d.ui.Event(es)
        self.canvas.Propagate(e)
        if e.NeedRedisplay():
            self.canvas.postRedisplay()
