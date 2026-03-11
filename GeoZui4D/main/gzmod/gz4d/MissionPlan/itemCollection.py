#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import gz4d
import xml.dom.minidom
import xml.xpath
import item

class ItemCollection(gz4d.OrderedDict):
    def __init__(self):
        gz4d.OrderedDict.__init__(self)
        self.gzTags = {}

    def Read(self, label, element):
        for t in xml.xpath.Evaluate(label,element):
            i = item.Item()
            i.Read(t)
            self[i.name] = i
            try:
                tag = i.attributes['gz']
                self.gzTags[i] = tag
            except KeyError:
                pass

    def PrettyString(self,level=0,tab='  '):
        ret = ''
        newLevel = level+1
        for k in self.keys():
            ret = ''.join((ret,newLevel*tab))
            try:
                ret= ''.join((ret,'(',self.gzTags[self[k]],')'))
            except KeyError:
                pass
            ret = ''.join((ret,str(k),': ',self[k].PrettyString(newLevel,tab)))
        ret = ''.join((ret,newLevel*tab,'gzTags:'))
        return ret


class ItemCollectionInstance(gz4d.OrderedDict):

    def __init__(self, items=None, profile=None, plan=None):
        gz4d.OrderedDict.__init__(self)
        self.gzTags = {}
        self.plan = plan
        if items:
            for k,i in items.iteritems():
                self[k] = item.ItemInstance(i,profile,plan)
                try:
                    t = items.gzTags[i]
                    self.gzTags[t]=self[k]
                except KeyError:
                    pass

    def Load(self,element):
        for k in self.keys():
            self[k].Load(element)

    def IsAnyEnabled(self):
        ret = False
        if self.items:
            for i in self.values():
                if i.enabled:
                    ret = True
                    break
        return ret

    def PrettyString(self,level=0,tab='  '):
        ret = ''
        newLevel = level+1
        for k in self.keys():
            ret = ''.join((ret,tab*newLevel))
            for t,i in self.gzTags.iteritems():
                if i is self[k]:
                    ret = ''.join((ret,'(',t,') '))
            ret = ''.join((ret,str(k),': ',self[k].PrettyString(level+1,tab)))
        return ret

    def updateDisplays(self):
        pos = self.plan.basePosition
        for i in self.values():
            ret = i.updateDisplay(pos)
            if ret is not None:
                pos = ret

    def findItem(self, iid):
        for i in self.values():
            ret = i.findItem(iid)
            if ret is not None:
                return ret
