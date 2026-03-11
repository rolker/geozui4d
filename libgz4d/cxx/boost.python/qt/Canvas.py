#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-

import gz4d

class Canvas(gz4d.ui.EventHandler):
    def __init__(self,parent):
        gz4d.ui.EventHandler.__init__(self)
        self.parent = parent

    def postRedisplay(self):
        self.parent.update()

