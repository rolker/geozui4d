#!/usr/bin/env python

import gz4d
import gz4d.qt

from PyQt4 import QtGui


class Application(gz4d.Application):
    def __init__(self,argv=[]):
        gz4d.Application.__init__(self)
        self.app = QtGui.QApplication(argv)
        self.mainWindow = QtGui.QMainWindow()
        self.gl = gz4d.qt.GLWidget(self.mainWindow)
        self.AddCanvas(self.gl.canvas)
        self.mainWindow.setCentralWidget(self.gl)

    def __call__(self):
        self.mainWindow.show()
        self.app.exec_()

