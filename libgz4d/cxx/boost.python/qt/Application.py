#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


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

