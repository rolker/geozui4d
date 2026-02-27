#!/usr/bin/env python
# -*- coding: utf-8 -*-

#import gz4d

class SixDOF:
    def __init__(self):
        self.position = [0.0,0.0,0.0]
        self.rotation = None

class PolhemusEvents:#(gz4d.ui.EventHandler):
    def __init__(self, fastrak):
        #gz4d.ui.EventHandler.__init__(self)
        self.fastrak = fastrak

    def __call__(self):
        p = self.fastrak.state[0]['position']
        o = self.fastrak.state[0]['orientation']

        if p is not None and o is not None:

            #sixdof = gz4d.ui.SixDOF()

            sixdof.position[0] = p[0]/100.0
            sixdof.position[1] = p[1]/100.0
            sixdof.position[2] = p[2]/100.0
            sixdof.orientation = gz4d.Rotationf(gz4d.Boost.quaternionf(o[0],o[1],o[2],o[3]))
            return  sixdof
