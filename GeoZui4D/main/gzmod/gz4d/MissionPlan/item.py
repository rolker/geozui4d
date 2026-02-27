#!/usr/bin/env python

import xml.dom.minidom
import xml.xpath

import itemBase
import itemCollection
import display
import gz4d

class Item(itemBase.ItemBase):
    '''
    An Item is a generic section of a profile.
    '''
    def __init__(self):
        itemBase.ItemBase.__init__(self)
        self.name = None
        self.items = None
        self.possibleValues = gz4d.OrderedDict()
        self.display=None

    def StringAttributes(self):
        '''
        Generator returning list of possible string attributes.
        '''
        for s in ('name','label','help','type','default','gz','path'):
            yield s

    def BoolAttributes(self):
        '''
        Generator returning list of possible flags.
        '''
        for b in ('attribute','element','required','restricted'):
            yield b

    def Read(self,element):
        '''
        Read our attributes from an xml node
        '''

        itemBase.ItemBase.Read(self,element)

        # see if we got named
        try:
            self.name = self.attributes['name']
        except KeyError:
            pass

        # look for boolean flags
        for b in self.BoolAttributes():
            if xml.xpath.Evaluate(''.join(('boolean(@',b,')')),element):
                self.attributes[b] = True

        # get sub items, if any
        if xml.xpath.Evaluate('boolean(item)',element):
            self.items = itemCollection.ItemCollection()
            self.items.Read('item',element)

        # get display, if we have one
        if xml.xpath.Evaluate('boolean(display)',element):
            self.display = display.Display()
            self.display.Read(xml.xpath.Evaluate('display',element)[0])

        # get list of possible values, if any
        for p in xml.xpath.Evaluate('possibleValue',element):
            self.possibleValues[(xml.xpath.Evaluate('string(text())',p))]=xml.xpath.Evaluate('string(@gz)',p)

    def PrettyString(self,level=0,tab='  '):
        ret = ''.join((itemBase.ItemBase.PrettyString(self,level,tab),(level+1)*tab,'Flags:'))
        for s in self.BoolAttributes():
            try:
                ret = ''.join((ret,' ',s,': ',str(self.attributes[s])))
            except KeyError:
                pass
        ret = ''.join((ret,'\n'))
        if self.display:
            ret = ''.join((ret,(level+1)*tab,'Display:',self.display.PrettyString(level,tab)))
        if self.possibleValues:
            ret = ''.join((ret,(level+1)*tab,'Possible Values: ',', '.join(self.possibleValues),'\n'))
        if self.items:
            ret = ''.join((ret,self.items.PrettyString(level,tab)))
        return ret


class ItemInstance:
    nextID = 1

    def __init__(self,item,profile,plan = None):
        self.enabled = False
        self.itemType = item
        self.userType = None
        self.items = None
        self.display = None
        self.profile = profile
        self.plan = plan
        self.iid = ItemInstance.nextID
        ItemInstance.nextID += 1

        try:
            self.value = item.attributes['default']
        except KeyError:
            self.value = None

        try: #we either have a type, or a collection of sub items
            t = item.attributes['type']
            try:
                self.userType = profile.types[t]
                self.items = itemCollection.ItemCollectionInstance(items=self.userType.items,profile=profile,plan=plan)
            except KeyError:
                # not a user defined type
                if t == 'bool':
                    self.value = not (self.value in ('0','false','off',None))
        except KeyError:
            if item.items:
                self.items = itemCollection.ItemCollectionInstance(items = item.items,profile=profile,plan=plan)

        if self.items:
            self.enabled = self.items.IsAnyEnabled()

        try:
            r = item.attributes['required']
            self.enabled = True
        except KeyError:
            pass

        self.initDisplay()


    def Load(self,element,thisNode=False):
        if not self.items:
            found = False
            try:
                if self.itemType.attributes['attribute']:
                    n = xml.xpath.Evaluate(''.join(('@',self.itemType.name)),element)
                    if n:
                        self.value = xml.xpath.Evaluate(''.join(('string(@',self.itemType.name,')')),element)
                        found = True
                        self.enabled = True
            except KeyError:
                pass

            try:
                if self.itemType.attributes['element']:
                    n = xml.xpath.Evaluate(self.itemType.name,element)
                    if n:
                        self.value = xml.xpath.Evaluate(''.join(('string(',self.itemType.name,'/text())')),element)
                        found = True
                        self.enabled = True
            except KeyError:
                pass

            try:
                if self.itemType.attributes['type'] == 'bool':
                    if found and self.value == self.profile.trueLabel:
                        self.value = True
                    else:
                        self.value = False
            except KeyError:
                pass
        else:
            if thisNode:
                c = element
            else:
                try:
                    p = self.itemType.attributes['path']
                except KeyError:
                    p = self.itemType.attributes['name']
                c = xml.xpath.Evaluate(p,element)
                if c:
                    c = c[0]
            if c:
                self.items.Load(c)
                self.enabled = (self.enabled or self.items.IsAnyEnabled())

        if self.display is not None:
            self.display.update()

    def PrettyString(self,level=0,tab='  '):
        ret = ' '.join(('type:',self.itemType.name,'enabled:',str(self.enabled)))
        if self.userType:
            ret = ' '.join((ret,'userType:',self.userType.name))
        if self.value:
            ret = ' '.join((ret,'value:',str(self.value)))
        ret = ''.join((ret,'\n'))
        if self.items:
            ret = ''.join((ret,self.items.PrettyString(level+1,tab)))
        return ret

    def initDisplay(self):
        if self.itemType.display is not None:
            self.startPositon = None
            self.display = self.itemType.display.createInstance(self,self.plan)

    def updateDisplay(self,pos):
        if self.display is not None:
            return self.display.update(pos)

    def setSelected(self,item):
        if self.display is not None:
            self.display.setSelected(item)

    def processCommand(self,commandNode):
        if commandNode.nodeName == 'update':
            for i in xml.xpath.Evaluate('position',commandNode):
                # a position in the update level rather than item level
                # means we are asking for gz cow position to inialize ourself
                for d in ('x','y','z'):
                    try:
                        self.items.gzTags[d].value = xml.xpath.Evaluate(d.join(('string(@',')')),i)
                    except KeyError:
                        pass
        if self.display is not None:
            self.display.processCommand(commandNode)

    def setValue(self,value):
        self.value = value
        self.enabled = True
        if self.plan is not None:
            if self.plan.gui is not None:
                self.plan.gui.ValueCallback(self)


    def findItem(self,itemID):
        if self.iid == itemID:
            return self
        if self.items is not None:
            return self.items.findItem(itemID)

