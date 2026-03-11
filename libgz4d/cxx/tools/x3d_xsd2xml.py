#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-

import xsd
import sys
import os
import xml.etree.ElementTree


class X3D:
    def __init__(self,fname):
        self.schema = xsd.Schema(fname)

        self.root = xml.etree.ElementTree.Element('X3D')
        self.root.text = '\n  '
        self.root.tail = '\n'
        indent = '  '

        for n in self.schema.children:
            if n.type == 'element':
                Element(n,self.root,indent)
            if n.type == 'complexType':
                BaseClass(n,self.root,indent)
            if n.type == 'simpleType':
                Type(n,self.root,indent)
            #if n.type == 'group':
                #Group(n,self.root)
            if n.type == 'attributeGroup':
                AttributeGroup(n,self.root,indent)

class ComplexTypeContainer:
    def __init__(self,node,element,indent):
        empty = True
        if node is not None:
            for c in node.children:
                if c.type == 'complexContent':
                    e = c.Get('extension')
                    if e is not None:
                        be = xml.etree.ElementTree.SubElement(element,'Base',{'type':e.attrib['base']})
                        be.tail = '\n'+indent+'  '
                        empty = False

                        for a in e.children:
                            if a.type == 'attribute':
                                Attribute(a,element,indent+'  ')
                                empty = False
                            if a.type in Grouping.types:
                                #Group(a,element)
                                Grouping(a,element,indent+'  ')
                                empty = False
                if c.type in Grouping.types:
                    Group(c,element,indent+'  ')
                    empty = False
        if empty:
            element.text = None

class Attribute:
    def __init__(self,node,parent,indent):
        e = xml.etree.ElementTree.SubElement(parent,'Field')
        e.tail = '\n'+indent

        if node.attrib.has_key('type'):
            e.attrib['type'] = node.attrib['type']
        else:
            st = node.Get('simpleType')
            e.text = '\n' + indent + '  '
            e.attrib['type'] = SimpleType(st,e,indent+'  ').type

        e.attrib['name'] = '_'.join(node.attrib['name'].split('-'))
        if node.attrib.has_key('default'):
            e.attrib['default'] = node.attrib['default']

        n = node.Get('annotation')
        if n is not None:
            Annotation(n,e,None)


class Restriction:
    def __init__(self,node,parent,indent):
        #e = xml.etree.ElementTree.SubElement(parent,'Restriction')
        #e.text = parent.text + '  '
        #e.tail = parent.text

        if node is not None:
            self.type = node.attrib['base']
            be = xml.etree.ElementTree.SubElement(parent,'Base',{'type':node.attrib['base']})
            be.tail = '\n'+indent+'  '

            n = node.Get('annotation')
            if n is not None:
                Annotation(n,be,indent+'  ')

            ee = None

            for c in node.children:
                if c.type == 'enumeration':
                    if ee is None:
                        ee = xml.etree.ElementTree.SubElement(parent,'Enumerations')
                        ee.text = '\n'+indent+'    '
                        ee.tail = '\n'+indent+'  '
                    ev = xml.etree.ElementTree.SubElement(ee,'Value')
                    ev.text = c.attrib['value']
                    ev.tail = '\n'+indent+'    '
                    n = c.Get('annotation')
                    if n is not None:
                        Annotation(n,ev,None)
                else:
                    r = xml.etree.ElementTree.SubElement(parent,'Restriction')
                    r.attrib['type'] = c.type
                    r.attrib['value'] = c.attrib['value']
                    r.tail = '\n'+indent+'  '




class SimpleType (Restriction):
    def __init__(self,node,parent,indent,element = None):
        if element is None:
            e = xml.etree.ElementTree.SubElement(parent,'Type')
            e.text = '\n' + indent + '  '
            e.tail = '\n' + indent
        else:
            e = element

        Restriction.__init__(self,node.Get('restriction'),e,indent)

def findContainer(index,nodeName):
    n = index[nodeName]
    if n.type == 'element':
        n = n.Get('complexType')
    if n.type == 'complexType':
        for c in n.children:
            if c.type == 'complexContent':
                e = c.Get('extension')
                if e is not None:
                    b = e.attrib['base']
                    for a in e.children:
                        if a.type == 'attribute' and a.attrib['name'] == 'containerField':
                            return a.attrib['default']
                    return findContainer(index,b)
    return ''

class Grouping:
    types = ('group','choice','sequence','attributeGroup')

    def __init__(self,node,parent,indent):
        self.getFields(node,parent,indent,'SFNode',[])

    def getFields(self,node,parent,indent,ntype,fnames):
        if node.attrib.has_key('maxOccurs'):
            ntype = 'MFNode'

        fnames = self.getFieldNames(node,fnames)

        if node.attrib.has_key('ref'):
            if node.type == 'element':
                for fn in fnames:
                    fn = '_'.join(fn.split('-'))
                    found = False
                    for f in parent.findall('Field'):
                        if f.get('name') == fn and f.get('type') == ntype:
                            if not node.attrib['ref'] in f.text.split(','):
                                fs = f.text.split(',')
                                fs.append(node.attrib['ref'])
                                f.text = ','.join(fs)
                            found = True
                    if not found:
                        fe = xml.etree.ElementTree.SubElement(parent,'Field')
                        fe.set('type',ntype)
                        fe.set('name',fn)
                        fe.text =  node.attrib['ref']
                        fe.tail = '\n'+indent
            else:
                self.getFields(node.index[node.attrib['ref']],parent,indent,ntype,fnames)

        for c in node.children:
            self.getFields(c,parent,indent,ntype,fnames)

    def getFieldNames(self,node,fn):
        n = node.Get('annotation')
        if n is not None:
            d = n.Get('documentation')
            if d is not None and d.text is not None:
                fn = [d.text]
                if ' or ' in d.text:
                    fn = d.text.split(' or ')
                if ', ' in d.text:
                    fn = d.text.split(', ')
        return fn


class Annotation:
    def __init__(self,node,parent,indent):
        self.element = xml.etree.ElementTree.SubElement(parent,'Documentation')
        if indent is not None:
            self.element.tail = '\n'+indent

        ai = node.Get('appinfo')
        if ai is not None:
            dox = xml.etree.ElementTree.SubElement(self.element,'Doxygen')
            dox.text = ai.text
            #dox.tail = self.element.tail

        d = node.Get('documentation')
        if d is not None:
            if d.text is not None:
                self.element.text = d.text
            if d.attrib.has_key('source'):
                self.element.attrib['source'] = d.attrib['source']

class Base:
    def __init__(self,node,element,indent):
        if node.attrib.has_key('name'):
            element.attrib['name'] = node.attrib['name']
        if node.attrib.has_key('ref'):
            element.attrib['ref'] = node.attrib['ref']
        an = node.Get('annotation')
        if an is not None:
            Annotation(an,element,indent+'  ')

class Element (Base,ComplexTypeContainer):
    def __init__(self,node, parent,indent):
        self.element = xml.etree.ElementTree.SubElement(parent,'Node')
        self.element.text = '\n'+indent+'  '
        self.element.tail = '\n'+indent

        Base.__init__(self,node,self.element,indent)
        ComplexTypeContainer.__init__(self,node.Get('complexType'),self.element,indent)

    def __str__(self):
        return Base.__str__(self) + ': ' + ComplexTypeContainer.__str__(self)

class BaseClass (Base,ComplexTypeContainer):
    def __init__(self,node,parent,indent):
        e = xml.etree.ElementTree.SubElement(parent,'Node')
        e.text = '\n'+indent+ '  '
        e.tail = '\n'+indent
        e.set('abstract','true')
        Base.__init__(self,node,e,indent)
        ComplexTypeContainer.__init__(self,node,e,indent)


class Type (Base,SimpleType):
    def __init__(self,node,parent,indent):
        e = xml.etree.ElementTree.SubElement(parent,'Type')
        e.text = '\n'+indent + '  '
        e.tail = '\n'+indent

        Base.__init__(self,node,e,indent)
        SimpleType.__init__(self,node,parent,indent,e)


class Group(Base,Grouping):
    def __init__(self,node,parent,indent):
        e = xml.etree.ElementTree.SubElement(parent,'Group')
        e.tail = '\n'+indent
        Base.__init__(self,node,e,indent)
        if not e.attrib.has_key('ref'):
            e.text = '\n' + indent + '  '
            Grouping.__init__(self,node,e,indent)

class AttributeGroup(Base,Grouping):
    def __init__(self,node,parent,indent):
        e = xml.etree.ElementTree.SubElement(parent,'AttributeGroup')
        e.text = '\n'+ indent + '  '
        e.tail = '\n'+ indent
        Base.__init__(self,node,e,indent)
        Grouping.__init__(self,node,e,indent)

        for a in node.children:
            if a.type == 'attribute':
                Attribute(a,e,indent+'  ')




def usage():
    print 'usage: x3d_xsd2xml.py [key=value ...]'
    print '     keys: out'
    sys.exit(1)


if __name__ == '__main__':
    params = {'out':'x3d_xsd.xml'}

    for a in sys.argv[1:]:
        if '=' in a:
            k,v = a.split('=',1)
            params[k] = v
        else:
            usage()

    x3d = X3D('../specs/x3d-3.2.xsd')

    out = xml.etree.ElementTree.ElementTree(x3d.root)
    out.write(params['out'])

