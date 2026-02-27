#!/usr/bin/env python

import gz4d

class PolhemusView(gz4d.scene.Viewpoint):
    def __init__(self, fastrak):
        self.fastrak = fastrak

    def GetMatrix(self):
        p = self.fastrak.state[0]['position']
        o = self.fastrak.state[0]['orientation']

        if p is not None and o is not None:
            x = (p[0]+15.0)/100.0
            y = (p[1]-18.0)/100.0
            z = p[2]/100.0

            return gz4d.Translationd(-x,-y,-z).GetMatrix() * gz4d.Rotationd(gz4d.Boost.quaterniond(o[0],o[1],o[2],o[3])).GetInverseMatrix()
        return gz4d.Matrix4d.Identity()

    def GetInverseMatrix(self):
        p = self.fastrak.state[0]['position']
        o = self.fastrak.state[0]['orientation']

        if p is not None and o is not None:
            x = (p[0]+15.0)/100.0
            y = (p[1]-18.0)/100.0
            z = p[2]/100.0

            return  gz4d.Rotationd(gz4d.Boost.quaterniond(o[0],o[1],o[2],o[3])).GetMatrix() * gz4d.Translationd(-x,-y,-z).GetInverseMatrix()

        return gz4d.Matrix4d.Identity()
