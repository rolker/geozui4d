#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import xsd
import sys
import os

class X3D:
    def __init__(self,fname):
        self.schema = xsd.Schema(fname)

        self.fieldTypes = []
        for c in self.schema.index['fieldTypeName'].children:
            if c.type == 'restriction':
                for t in c.children:
                    self.fieldTypes.append(t.attrib['value'])

        self.elements = {}
        self.baseClasses = {}
        self.types = {}
        self.groups = {}
        self.aGroups = {}

        for n in self.schema.children:
            if n.type == 'element':
                self.elements[n.attrib['name']]=Element(n.attrib['name'],self.schema)
            if n.type == 'complexType':
                self.baseClasses[n.attrib['name']]=BaseClass(n.attrib['name'],self.schema)
            if n.type == 'simpleType':
                self.types[n.attrib['name']]=Type(n.attrib['name'],self.schema)
            if n.type == 'group':
                self.groups[n.attrib['name']]=Group(n.attrib['name'],self.schema)
            if n.type == 'attributeGroup':
                self.aGroups[n.attrib['name']]=AttributeGroup(n.attrib['name'],self.schema)

class ComplexTypeContainer:
    def __init__(self,node):
        self.base = None
        self.attributes = []
        self.groups = []

        if node is not None:
            for c in node.children:
                if c.type == 'complexContent':
                    e = c.Get('extension')
                    if e is not None:
                        self.base = e.attrib['base']
                        for a in e.children:
                            if a.type == 'attribute':
                                self.attributes.append(Attribute(a))
                            if a.type in Grouping.types:
                                self.groups.append(Grouping(a))
                if c.type in Grouping.types:
                    self.groups.append(Grouping(c))

    def __str__(self):
        ret = str(self.base) + ' {\n'
        for a in self.attributes:
            ret += '\t'+str(a)+'\n'
        ret += '}\n'
        return ret

    def getContainerField(self,x3d):
        for a in self.attributes:
            if a.name == 'containerField':
                return a.default
        if self.base in x3d.elements.keys():
            return x3d.elements[self.base].getContainerField(x3d)
        if self.base in x3d.baseClasses.keys():
            return x3d.baseClasses[self.base].getContainerField(x3d)


class Attribute:
    def __init__(self,node):
        if node.attrib.has_key('type'):
            self.type = node.attrib['type']
        else:
            st = node.Get('simpleType')
            self.type = SimpleType(st)
        if self.type == 'xs:NMTOKEN':
            self.type = 'SFString'

        self.name = node.attrib['name']
        if node.attrib.has_key('default'):
            self.default = node.attrib['default']
        else:
            self.default = None

    def __str__(self):
        return ' '.join((str(self.type), self.name)) 

    def cname(self):
        if self.name == 'class':
            return 'class_'
        return '_'.join(self.name.split('-'))

    def ctype(self,x3d):
        t = self.type

        if t in x3d.fieldTypes:
            return t

        if x3d.types.has_key(t):
            return x3d.types[t].cbase()

        try:
            if len(t) > 3 and t[:3] == 'xs:':
                return 'SFString'
        except AttributeError:
            return t.cbase()



    def cinit(self):
        if self.default is not None:
            if '"' in self.default:
                d = '\\"'.join(self.default.split('"'))
            else:
                d = self.default
            return self.name+'("'+d+'")'

class Restriction:
    def __init__(self,node):
        self.base = None
        self.enums = []
        if node is not None:
            self.base = node.attrib['base']

            for e in node.children:
                if e.type == 'enumeration':
                    self.enums.append(e.attrib['value'])

    def __str__(self):
        ret = str(self.base)
        if len(self.enums):
            ret += ' (e: ' + ','.join(self.enums) + ')'
        return ret

    def cbase(self):
        if len(self.base) > 3 and self.base[:3] == 'xs:':
            return 'SFString'
        return self.base


class SimpleType (Restriction):
    def __init__(self,node):
        Restriction.__init__(self,node.Get('restriction'))

    def __str__(self):
        return Restriction.__str__(self)

class Grouping:
    types = ('element','group','choice','sequence','attributeGroup')

    def __init__(self,node):
        self.type = node.type
        self.contents = []
        if node.attrib.has_key('ref'):
            self.ref = node.attrib['ref']
        else:
            self.ref = None

        n = node.Get('annotation')
        if n is not None:
            self.notes = Annotation(n)
        else:
            self.notes = None

        for c in node.children:
            if c.type in Grouping.types:
                self.contents.append(Grouping(c))

    def header(self,x3d):
        ret = []
        if self.ref is not None:
            if self.type == 'element':
                container = x3d.elements[self.ref].getContainerField(x3d)
                if self.notes is not None:
                    if self.notes.documentation is not None and ' ' not in self.notes.documentation:
                        container = self.notes.documentation
                ret.append((self.ref, container))
            else:
                if self.type == 'attributeGroup':
                    for a in x3d.aGroups[self.ref].attributes:
                        ret.append((a.ctype(x3d),a.cname()))
                else:
                    ret += x3d.groups[self.ref].header(x3d)
        for c in self.contents:
            ret += c.header(x3d)
        return ret

class Annotation:
    def __init__(self,node):
        d = node.Get('documentation')
        if d is not None:
            self.documentation = d.text
        else:
            self.documentation = None

class Base:
    def __init__(self,name,schema):
        self.name = name
        self.schema = schema
        self.schema_node = schema.index[name]

    def __str__(self):
        return self.name

class Element (Base,ComplexTypeContainer):
    def __init__(self,name,schema):
        Base.__init__(self,name,schema)
        ComplexTypeContainer.__init__(self,self.schema_node.Get('complexType'))

    def __str__(self):
        return Base.__str__(self) + ': ' + ComplexTypeContainer.__str__(self)

class BaseClass (Base,ComplexTypeContainer):
    def __init__(self,name,schema):
        Base.__init__(self,name,schema)
        ComplexTypeContainer.__init__(self,self.schema_node)

    def __str__(self):
        return Base.__str__(self) + ': ' + ComplexTypeContainer.__str__(self)


class Type (Base,SimpleType):
    def __init__(self,name,schema):
        Base.__init__(self,name,schema)
        SimpleType.__init__(self,self.schema_node)

    def __str__(self):
        return Base.__str__(self) + ': ' + SimpleType.__str__(self)

class Group(Base,Grouping):
    def __init__(self,name,schema):
        Base.__init__(self,name,schema)
        Grouping.__init__(self,self.schema_node)

class AttributeGroup(Base,Grouping):
    def __init__(self,name,schema):
        Base.__init__(self,name,schema)
        self.attributes = []
        for a in self.schema_node.children:
            if a.type == 'attribute':
                self.attributes.append(Attribute(a))


def writeHeader(hdir,c,x3d):
    fname = c.name+'.h'
    out = open(os.path.join(hdir,fname),'w')

    out.write('#ifndef GZ4D_X3D_'+c.name.upper()+'_H\n')
    out.write('#define GZ4D_X3D_'+c.name.upper()+'_H\n')
    out.write('\n')
    out.write('#include<gz4d/x3d/base.h>\n')
    if c.base is not None:
        out.write('#include<gz4d/x3d/ng/'+c.base+'.h>\n')
    out.write('\n')
    out.write('namespace gz4d\n')
    out.write('{\n')
    out.write('    namespace x3d\n')
    out.write('    {\n')
    out.write('        class '+c.name)
    if c.base is not None:
        out.write(': public '+c.base)
    out.write('\n')
    out.write('        {\n')

    for g in n.groups:
        attribs = {}
        for a in g.header(x3d):
            if not attribs.has_key(a[1]):
                attribs[a[1]] = []
            attribs[a[1]].append(a[0])
        for l,t in attribs.iteritems():
            if l is not None:
                out.write('            SFNode '+l+'; //'+','.join(t)+'\n')
    for a in n.attributes:
        out.write('            '+a.ctype(x3d)+' '+a.cname()+';\n')
    out.write('            public:\n')
    out.write('                '+c.name+'();\n')
    out.write('                virtual void Load(xml::Node x3d);\n')
    out.write('        };\n')
    out.write('    }\n')
    out.write('}\n')
    out.write('\n')
    out.write('#endif\n')
    out.write('\n')

def writeImplementation(cdir,c,x3d):
    fname = c.name+'.cpp'
    out = open(os.path.join(cdir,fname),'w')

    out.write('#define GZ4D_INTERNAL\n')
    out.write('#include <gz4d/x3d/ng/'+c.name+'.h>\n')
    out.write('\n')
    out.write('gz4d::x3d::'+c.name+'::'+c.name+'()\n')
    inits = []
    for a in n.attributes:
        i = a.cinit()
        if i is not None:
            inits.append(i)
    if len(inits):
        out.write('    :'+','.join(inits)+'\n')

    out.write('{\n')
    out.write('}\n')

    out.write('\n')

    out.write('void gz4d::x3d::'+c.name+'::'+'Load(xml::Node x3d)\n')
    out.write('{\n')
    if c.base is not None:
        out.write('    '+c.base+'::Load(x3d);\n')
    for a in n.attributes:
        out.write('    '+a.cname()+'.Load(x3d,"'+a.name+'");\n')
    out.write('}\n')

    out.write('\n')



def usage():
    print 'usage: xsd_X3D.py [key=value ...]'
    print '     keys: specs,hdir,cdir'
    sys.exit(1)


if __name__ == '__main__':
    params = {}

    for a in sys.argv[1:]:
        if '=' in a:
            k,v = a.split('=',1)
            params[k] = v
        else:
            usage()

    x3d = X3D(params['specs'])

    print '\n***** Types *****\n'
    for t in x3d.types:
        print t

    print '\n***** Elements *****\n'
    for e in x3d.elements:
        print e

    print '\n***** Base classes *****\n'
    for b in x3d.baseClasses:
        print b

    for n in x3d.baseClasses.itervalues():
        writeHeader(params['hdir'],n,x3d)
        writeImplementation(params['cdir'],n,x3d)

    for n in x3d.elements.itervalues():
        writeHeader(params['hdir'],n,x3d)
        writeImplementation(params['cdir'],n,x3d)

