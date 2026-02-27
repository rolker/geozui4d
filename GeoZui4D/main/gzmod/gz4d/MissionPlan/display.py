#!/usr/bin/env python

import itemBase
import math
import xml.dom.minidom
import xml.xpath

class Display(itemBase.ItemBase):
    '''
    Base class for 3D display types
    '''
    def __init__(self):
        itemBase.ItemBase.__init__(self)

    def StringAttributes(self):
        '''
        Generator returning list of possible string attributes.
        '''
        yield 'type'

    def createInstance(self, item, plan=None):
        for dname,dclass in (('goto',GotoDisplay), ('home',HomeDisplay), ('wait',WaitDisplay), ('grid',GridDisplay), ('leg',LegDisplay)):
            if self.attributes['type'] == dname:
                return dclass(item,plan)
        return None


class DisplayInstance:

    def __init__(self, item, plan=None):
        self.item = item
        self.plan = plan
#        print 'DisplayInstance::init'
#        print '  item:',item.PrettyString(level=1)
#        print '  type:',item.itemType.PrettyString(level=1)
        self.sendNew()

    def update(self, pos):
        position = self.getPosition()
        command = '<mp>'
        command = ''.join((command,'<update id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0">',self.getPositionTag(position),'</item>'))
        command = ''.join((command,'</update>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)

    def getPosition(self):
        return [self.getFloat('x'), self.getFloat('y'), self.getFloat('z')]

    def getFloat(self,label):
        try:
            return float(self.item.items.gzTags[label].value)
        except TypeError:
            return 0.0

    def setValue(self,label,value):
        try:
            self.item.items.gzTags[label].setValue(value)
        except KeyError:
            pass

    def setSelected(self,item):
        self.sendCommand('"'.join(('<mp><select id=',str(self.item.iid),'><item id=',str(item),'/></select></mp>')))

    def sendNew(self):
        command = '<mp>'
        command = ''.join((command,'<newcmd id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0">',self.getColorTag(),'</item>'))
        command = ''.join((command,'</newcmd>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)


    def sendCommand(self,command):
        if (self.plan is not None) and (self.plan.gzmp is not None):
            self.plan.gzmp.sendData(command)

    def getColor(self):
        return (0.7,0.7,0.7)

    def getSelectedColor(self):
        return (1.0,1.0,1.0)

    def getPositionTag(self,position,pid=None):
        #if self.plan is not None:
            #p = map(lambda a, b: float(a)-float(b), position, self.plan.basePosition)
        #else:
            #p = position
        p = position
        if pid is not None:
            pidstr = '"'.join((' id=',str(pid),''))
        else:
            pidstr = ''

        return ''.join(('<position x="',str(p[0]),'" y="',str(p[1]),'" z="',str(p[2]),'"',pidstr,'/>'))

    def getColorTag(self):
        c = self.getColor()
        sc = self.getSelectedColor()
        return ''.join(('<color r="',str(c[0]),'" g="',str(c[1]),'" b="',str(c[2]),'"/><selectedColor r="',str(sc[0]),'" g="',str(sc[1]),'" b="',str(sc[2]),'"/>'))

    def processCommand(self,commandNode):
        if commandNode.nodeName == 'update':
            for i in xml.xpath.Evaluate('item',commandNode):
                iid =  xml.xpath.Evaluate('number(@id)',i)
                if iid == 0:
                    for p in xml.xpath.Evaluate('position',i):
                        for c in ('x','y','z'):
                            self.setValue(c,xml.xpath.Evaluate('number(@'+c+')',p))
                if iid == 1:
                    for p in xml.xpath.Evaluate('position',i):
                        for c in ('ex','ey','ez'):
                            self.setValue(c,xml.xpath.Evaluate('number(@'+c[:-1]+')',p))


class GotoDisplay(DisplayInstance):

    def sendNew(self):
        command = '<mp>'
        command = ''.join((command,'<newcmd id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0" size=".5"><color r=".5" g=".5" b=".6"/><selectedColor r=".8" g=".8" b="1"/></item>'))
        command = ''.join((command,'<item type="segment" id="1" size=".3"><color r=".4" g=".4" b=".4"/><selectedColor r=".7" g=".7" b=".8"/></item>'))
        command = ''.join((command,'</newcmd>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)

    def processCommand(self,commandNode):
        if commandNode.nodeName == 'update':
            for i in xml.xpath.Evaluate('item',commandNode):
                iid =  xml.xpath.Evaluate('number(@id)',i)
                if iid == 0:
                    for p in xml.xpath.Evaluate('position',i):
                        for c in ('x','y','z'):
                            self.setValue(c,xml.xpath.Evaluate('number(@'+c+')',p))

    def update(self, position):
        position = self.getPosition()

        command = '<mp>'
        command = ''.join((command,'<update id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item id="0">',self.getPositionTag(position),'</item>'))
        if position is not None:
            command = ''.join((command,'<item id="1">',self.getPositionTag(sposition,0),self.getPositionTag(position,1),'</item>'))
        command = ''.join((command,'</update>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)
        return position[:]

class HomeDisplay(DisplayInstance):
    pass

class WaitDisplay(DisplayInstance):

    def update(self,cmd = 'update'):
        pass
#        if self.plan is not None and self.plan.gzmp is not None:
#            command = ''.join(('<mp><',cmd,' id="',str(self.item.cid),'"><handle id="0">',self.getPositionTag(self.endPosition),'</handle></',cmd,'></mp>'))
#            self.plan.gzmp.sendData(command)

    def sendNew(self):
        command = '<mp>'
        command = ''.join((command,'<newcmd id="',str(self.item.iid),'">'))
        command = ''.join((command,'<point id="0">',self.getColorTag(),'</point>'))
        command = ''.join((command,'</newcmd>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)


class GridDisplay(DisplayInstance):

    def update(self,cmd = 'update'):
        position = self.getPosition()

        l = self.getFloat('l')
        w = self.getFloat('w')
        o = math.radians(self.getFloat('o'))

        eposition = [position[0] + w*math.cos(o) -l*math.sin(o),position[1] + w*math.sin(o) +l*math.cos(o),position[2]]

        command = '<mp>'
        command = ''.join((command,'<update id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0">',self.getPositionTag(position),'</item>'))
        command = ''.join((command,'<item type="handle" id="1">',self.getPositionTag(eposition),'</item>'))
        command = ''.join((command,'</update>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)


    def sendNew(self):
        command = '<mp>'
        command = ''.join((command,'<newcmd id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0">',self.getColorTag(),'</item>'))
        command = ''.join((command,'<item type="handle" id="1">',self.getColorTag(),'</item>'))
        command = ''.join((command,'</newcmd>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)

class LegDisplay(DisplayInstance):

    def update(self,cmd = 'update'):
        position = self.getPosition()
        eposition = [self.getFloat('ex'),self.getFloat('ey'),self.getFloat('ez')]

        command = '<mp>'
        command = ''.join((command,'<update id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item id="0">',self.getPositionTag(position),'</item>'))
        command = ''.join((command,'<item id="1">',self.getPositionTag(eposition),'</item>'))
        command = ''.join((command,'<item id="2">',self.getPositionTag(position,0),self.getPositionTag(eposition,1),'</item>'))
        command = ''.join((command,'</update>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)


    def sendNew(self):
        command = '<mp>'
        command = ''.join((command,'<newcmd id="',str(self.item.iid),'">'))
        command = ''.join((command,'<item type="handle" id="0" size=".5"><color r=".5" g=".5" b=".6"/><selectedColor r=".9" g=".9" b="1"/></item>'))
        command = ''.join((command,'<item type="handle" id="1" size=".5"><color r=".5" g=".5" b=".6"/><selectedColor r=".9" g=".9" b="1"/></item>'))
        command = ''.join((command,'<item type="segment" id="2" size=".25"><color r=".5" g=".5" b=".6"/><selectedColor r=".9" g=".9" b="1"/></item>'))
        command = ''.join((command,'</newcmd>'))
        command = ''.join((command,'</mp>'))
        self.sendCommand(command)

    def processCommand(self,commandNode):
        DisplayInstance.processCommand(self,commandNode)
        self.update()
