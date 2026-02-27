#!/usr/bin/env python
# -*- coding: utf-8 -*-

import gz4d

class Canvas(gz4d.ui.EventHandler):
    def __init__(self,parent):
        gz4d.ui.EventHandler.__init__(self)
        self.parent = parent

    def postRedisplay(self):
        self.parent.update()

