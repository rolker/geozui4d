#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# -*- coding: utf-8 -*-
# coding: utf-8

import xml.etree.ElementTree
import sys
import os

typemap = '''
Node         boost::shared_ptr<gz4d::x3d::X3DNode>
Bool         bool
Color        gz4d::Vector<float,3>
ColorRGBA    gz4d::Vector<float,4>
Double       double
Float        float
Image        gz4d::x3d::Image
Int32        int32_t
String       std::string
Time         gz4d::Time
Vec2d        gz4d::Vector<double,2>
Vec3d        gz4d::Vector<double,3>
Vec4d        gz4d::Vector<double,4>
Vec2f        gz4d::Vector<float,2>
Vec3f        gz4d::Vector<float,3>
Vec4f        gz4d::Vector<float,4>
Rotation     gz4d::Vector<float,4>
Matrix3f     gz4d::Matrix<float,3,3>
Matrix4f     gz4d::Matrix<float,4,4>
'''


class Class:
    def __init__(self,parent,indent,name,bases=None):
        self.indent = indent + '  '
        self.element = xml.etree.ElementTree.SubElement(parent,'Class')
        self.element.text = self.indent
        self.element.tail = indent
        

        self.element.attrib['name'] = name
        if bases is not None:
            self.element.attrib['bases']= bases

        self.dec = None
        self.imp = None

        self.scope = self.element
        self.scope_indent = self.indent

    def setNamespace(self,ns):
        self.element.attrib['namespace'] = ns

    def setIncludePrefix(self,pre):
        self.element.attrib['include_prefix'] = pre

    def addDeclarationHeader(self,h):
        if self.dec is None:
            self.dec = xml.etree.ElementTree.SubElement(self.element,'Declaration')
            self.dec.text = self.indent + '  '
            self.dec.tail = self.indent

        he = xml.etree.ElementTree.SubElement(self.dec,'Header')
        he.text = h
        he.tail = self.indent + '  '

    def addForwardDeclaration(self,fwd):
        if self.dec is None:
            self.dec = xml.etree.ElementTree.SubElement(self.element,'Declaration')
            self.dec.text = self.indent + '  '
            self.dec.tail = self.indent

        fwde = xml.etree.ElementTree.SubElement(self.dec,'ForwardDeclaration')
        fwde.text = fwd
        fwde.tail = self.indent + '  '

    def addImplementationHeader(self,h):
        if self.imp is None:
            self.imp = xml.etree.ElementTree.SubElement(self.element,'Implementation')
            self.imp.text = self.indent + '  '
            self.imp.tail = self.indent

        he = xml.etree.ElementTree.SubElement(self.imp,'Header')
        he.text = h
        he.tail = self.indent + '  '

    def setScope(self,s):
        self.scope = xml.etree.ElementTree.SubElement(self.element,s)
        self.scope.text = self.indent + '  '
        self.scope.tail = self.indent
        self.scope_indent = self.indent + '  '


    def addVariable(self,t,name,static=False):
        v = Variable(self.scope,self.scope_indent,t,name,static)
        return v

    def addTypedef(self,t,name):
        td = xml.etree.ElementTree.SubElement(self.scope,'Typedef')
        td.text = self.scope_indent + '  '
        td.tail = self.scope_indent
        td.attrib['type'] = t
        td.attrib['name'] = name

    def addEnum(self,name):
        e = Enum(self.scope,self.scope_indent,name)
        return e

    def addFunction(self,t,name,const=False,virtual=False,static=False):
        f = Function(self.scope,self.scope_indent,t,name,const,virtual,static)
        return f

    def addConstructor(self,virtual=False):
        c = Constructor(self.scope,self.scope_indent,virtual)
        return c

    def addDocumentation(self,d):
        ret = Documentation(self.scope,self.scope_indent,d)
        return ret


class FunctionBase:
    def __init__(self,etype,parent,indent,t,name,const=False,virtual=False,static=False):
        self.indent = indent + '  '
        self.element = xml.etree.ElementTree.SubElement(parent,etype)
        self.element.text = self.indent
        self.element.tail = indent
        if t is not None:
            self.element.attrib['type'] = t
        if name is not None:
            self.element.attrib['name'] = name
        if const:
            self.element.attrib['const'] = 'true'
        if virtual:
            self.element.attrib['virtual'] = 'true'
        if static:
            self.element.attrib['static'] = 'true'

    def addParameter(self,t,name):
        p = xml.etree.ElementTree.SubElement(self.element,'Parameter')
        #p.text = self.element.text + '  '
        p.tail = self.indent
        p.attrib['type'] = t
        p.attrib['name'] = name

    def addImplementation(self,imp):
        fi = xml.etree.ElementTree.SubElement(self.element,'Implementation')
        fi.tail = self.indent
        fi.text = imp

    def addDocumentation(self,d):
        ret = Documentation(self.element,d)
        return ret


class Constructor (FunctionBase):
    def __init__(self,parent,indent,virtual=False):
        FunctionBase.__init__(self,'Constructor',parent,indent,None,None,False,virtual)

    def addInitializer(self,i):
        fi = xml.etree.ElementTree.SubElement(self.element,'Initializer')
        fi.tail = self.indent
        fi.text = i

class Function (FunctionBase):
    def __init__(self,parent,indent,t,name,const=False,virtual=False,static=False):
        FunctionBase.__init__(self,'Function',parent,indent,t,name,const,virtual,static)

class Enum:
    def __init__(self,parent,indent,name):
        self.indent = indent + '  '
        self.element = xml.etree.ElementTree.SubElement(parent,'Enum')
        self.element.text = self.indent
        self.element.tail = indent
        self.element.attrib['name'] = name

    def addValue(self,value):
        v = xml.etree.ElementTree.SubElement(self.element,'Value')
        v.text = value
        v.tail = self.indent

    def addDocumentation(self,d):
        ret = Documentation(self.element,self,indent,d)
        return ret

class Variable:
    def __init__(self,parent,indent,t,name,static):
        self.indent = indent + '  '
        self.element = xml.etree.ElementTree.SubElement(parent,'Variable')
        self.element.text = self.indent
        self.element.tail = indent
        self.element.set('type',t)
        self.element.set('name',name)
        if static:
            self.element.set('static','true')

    def addImplementation(self,imp):
        fi = xml.etree.ElementTree.SubElement(self.element,'Implementation')
        fi.tail = self.indent
        fi.text = imp

    def addDocumentation(self,d):
        ret = Documentation(self.element,self.indent,d)
        return ret

class Documentation:
    def __init__(self,parent,indent,d):
        self.indent = indent+'  '
        self.element = xml.etree.ElementTree.SubElement(parent,'Documentation')
        self.element.text = d
        self.element.tail = indent

    def setSource(self,s):
        self.element.set('source',s)


def Document(c,n,name):
    d = n.find('Documentation')
    if d is not None:
        ds = name
        dox = d.find('Doxygen')
        if dox is not None and dox.text is not None:
            ds = dox.text
        doc = c.addDocumentation(ds)
        if d.attrib.has_key('source'):
            doc.setSource(d.attrib['source'])

def GenerateFields(parent,indent,root):

    for t in typemap.splitlines():
        if len(t):
            xtype,ctype = t.split(None,1)

            c = Class(parent,indent,'SF'+xtype)
            c.setNamespace('gz4d::x3d')
            c.setIncludePrefix('gz4d/x3d/ng')

            for n in root.findall('Type'):
                if n.attrib['name'] == 'SF'+xtype:
                    Document(c,n,'SF'+xtype)
                    break

            for h in ('gz4d/x3d/FieldValue.h','gz4d/Vector.h','gz4d/time.h','gz4d/Matrix.h','gz4d/x3d/Image.h'):
                c.addDeclarationHeader(h)

            c.addForwardDeclaration('class X3DNode;');

            for h in ('sstream','boost/algorithm/string.hpp','gz4d/vector/ios.h','gz4d/time/ios.h'):
                c.addImplementationHeader(h)

            c.addVariable(ctype,'value')

            c.setScope('public')

            c.addConstructor() #default constructor

            con = c.addConstructor() #constructor with string param
            con.addParameter('std::string const &','s')
            con.addImplementation('*this = s;\n')

            f = c.addFunction('void','operator=')
            f.addParameter('std::string const &','s')

            imp = None
            if xtype != 'Node':
                imp = 'std::istringstream is(s);\n'
                imp += 'is >> value;\n'
            if xtype == 'Bool':
                imp = 'value = (s == "true");\n'
            f.addImplementation(imp)

            if ctype != 'std::string':
                f = c.addFunction('void','operator=')
                f.addParameter(ctype + ' const &','v')
                f.addImplementation('value = v;\n')

            f = c.addFunction('','operator '+ctype+' &')
            f.addImplementation('return value;\n')

            f = c.addFunction('','operator '+ctype+' const &',const=True)
            f.addImplementation('return value;\n')

            f = c.addFunction('std::string','getString')
            f.addParameter('std::string const &','indent')
            gsimp  = 'std::stringstream ret;\n'
            if xtype == 'Node':
                gsimp += 'if (value)\n';
                gsimp += '    ret << value->getString("");\n'
                gsimp += 'else\n';
                gsimp += '    ret << "(null)";\n'
                c.addImplementationHeader('gz4d/x3d/ng/X3DNode.h')
            else:
                gsimp += 'ret << value;\n'
            gsimp += 'return ret.str();\n'
            f.addImplementation(gsimp)


            c = Class(parent,indent,'MF'+xtype)
            c.setNamespace('gz4d::x3d')
            c.setIncludePrefix('gz4d/x3d/ng')

            for n in root.findall('Type'):
                if n.attrib['name'] == 'MF'+xtype:
                    Document(c,n,'MF'+xtype)
                    break

            for h in ('gz4d/Vector.h','gz4d/time.h','gz4d/Matrix.h','gz4d/x3d/Image.h','gz4d/x3d/ng/X3DNode.h'):
                c.addDeclarationHeader(h)

            for h in ('sstream','boost/algorithm/string.hpp','gz4d/vector/ios.h','gz4d/time/ios.h'):
                c.addImplementationHeader(h)

            c.addVariable('std::vector<'+ctype+' >','values')

            c.setScope('public')

            c.addConstructor() #default constructor

            con = c.addConstructor() #constructor with string param
            con.addParameter('std::string const &','s')
            con.addImplementation('*this = s;\n')


            c.addTypedef('std::vector<'+ctype+' >::iterator','iterator')
            c.addTypedef('std::vector<'+ctype+' >::const_iterator','const_iterator')

            f = c.addFunction('void','operator=')
            f.addParameter('std::string const &','s')

            imp = None

            if xtype != 'Node':
                imp =  'std::vector<std::string> parts;\n'
                imp += 'boost::split(parts,s,boost::algorithm::is_any_of(","));\n'
                imp += 'if (parts.size() >= 2)\n'
                imp += '{\n'
                imp += '    for(std::vector<std::string>::iterator i = parts.begin(); i != parts.end(); ++i)\n'
                imp += '    {\n'
                imp += '        std::istringstream datastrm(*i);\n'
                imp += '        if(datastrm)\n'
                imp += '        {\n'
                imp += '            '+ctype+' v;\n'
                imp += '            datastrm >> v;\n'
                imp += '            values.push_back(v);\n'
                imp += '        }\n'
                imp += '    }\n'
                imp += '}\n'
                imp += 'else\n'
                imp += '{\n'
                imp += '    std::istringstream datastrm(s);\n'
                imp += '    while(datastrm.peek()>=0) //there were no commas, so assume the user put in the right amount in the right places!\n'
                imp += '    {\n'
                imp += '        '+ctype+' v;\n'
                imp += '        datastrm >> v;\n'
                imp += '        values.push_back(v);\n'
                imp += '    }\n'
                imp += '}\n'

            f.addImplementation(imp)


            f = c.addFunction('void','append')
            f.addParameter(ctype+' const &','v')
            f.addImplementation('values.push_back(v);\n')

            f = c.addFunction('std::vector<'+ctype+' > const *','operator->')
            f.addImplementation('return &values;\n')

            f = c.addFunction('std::vector<'+ctype+' > const &','operator*')
            f.addImplementation('return values;\n')

            f = c.addFunction('gz4d::x3d::MF'+xtype+'::const_iterator','begin',const=True)
            f.addImplementation('return values.begin();\n')

            f = c.addFunction('gz4d::x3d::MF'+xtype+'::iterator','begin')
            f.addImplementation('return values.begin();\n')

            f = c.addFunction('gz4d::x3d::MF'+xtype+'::const_iterator','end',const=True)
            f.addImplementation('return values.end();\n')

            f = c.addFunction('gz4d::x3d::MF'+xtype+'::iterator','end')
            f.addImplementation('return values.end();\n')

            f = c.addFunction('bool','empty',const=True)
            f.addImplementation('return values.empty();\n')

            f = c.addFunction('std::size_t','size',const=True)
            f.addImplementation('return values.size();\n')

            f = c.addFunction('std::string','getString')
            f.addParameter('std::string const &','indent')
            gsimp  = 'std::stringstream ret;\n'
            gsimp += 'ret << values.size() << " elements\\n";\n'
            gsimp += 'ret << indent << "(";\n'
            gsimp += 'for (iterator i = values.begin(); i != values.end(); ++i)\n'
            if xtype == 'Node':
                gsimp += '    if (*i)\n'
                gsimp += '        ret << "\\n" << (*i)->getString(indent+"\\t");\n'
                gsimp += '    else\n'
                gsimp += '        ret << "\\n" << indent << "\\t(null)";\n'
            else:
                gsimp += '    {\n'
                gsimp += '        if(i != values.begin())\n'
                gsimp += '            ret << ", ";\n'
                gsimp += '        ret << *i;\n'
                gsimp += '    }\n'
            gsimp += 'ret << ")";\n'
            gsimp += 'return ret.str();\n'
            f.addImplementation(gsimp)


def GenerateNode(node,parent,indent,agroups):
    name = node.attrib['name']
    base = None
    for c in node.getchildren():
        if c.tag == 'Base':
            base = c.attrib['type']

    b = base
    c = Class(parent,indent,name,b)
    c.setNamespace('gz4d::x3d')
    c.setIncludePrefix('gz4d/x3d/ng')

    if b is not None:
        c.addDeclarationHeader('gz4d/x3d/ng/'+base+'.h')
    else:
        c.addDeclarationHeader('gz4d/gz4d.h')
        c.addDeclarationHeader('string')
        c.addDeclarationHeader('gz4d/x3d/FieldValue.h')

    c.addForwardDeclaration('class SFNode;')
    c.addImplementationHeader('gz4d/x3d/ng/SFNode.h')

    Document(c,node,name)


    types = []

    fields = []

    for g in node.findall('Group'):
        if g.attrib.has_key('ref'):
            for ag in agroups:
                if ag.attrib['name'] == g.attrib['ref']:
                    for f in ag.findall('Field'):
                        fname = f.attrib['name']
                        if fname != '' and fname != 'class':
                            fields.append({'name':fname, 'type':f.attrib['type'], 'doc':[]})
                            if f.attrib.has_key('default'):
                                fields[-1]['default'] = f.attrib['default']
                                fields[-1]['doc'].append('Default: ' + f.attrib['default'])

                            t = f.find('Type')
                            if t is not None:
                                for r in t.findall('Restriction'):
                                    fields[-1]['doc'].append('Restriction '+r.attrib['type']+': '+r.attrib['value'])


    for f in node.findall('Field'):
        fname = f.attrib['name']
        if fname != '':
            fields.append({'name':fname, 'type':f.attrib['type'], 'doc':[]})
            if f.attrib.has_key('default'):
                fields[-1]['default'] = f.attrib['default']
                fields[-1]['doc'].append('Default: ' + f.attrib['default'])

                t = f.find('Type')
                if t is not None:
                    for r in t.findall('Restriction'):
                        fields[-1]['doc'].append('Restriction '+r.attrib['type']+': '+r.attrib['value'])



    for f in fields:
        if not f['type'] in types:
            types.append(f['type'])

        fv = c.addVariable(f['type'],'_'+f['name'])
        if len(f['doc']):
            fv.addDocumentation('\n'.join(f['doc']))


    if name == 'X3DNode':
        c.addVariable('SFString','_containerField')

    #for f in node.findall('Field'):
        #fname = f.attrib['name']
        #if fname != '':
            #ftype = f.attrib['type']
            #if not ftype in types:
                #types.append(ftype)

            #c.addVariable(ftype,'_'+fname)

    for t in types:
        c.addDeclarationHeader('gz4d/x3d/ng/'+t+'.h')

    c.setScope('public')
    con = c.addConstructor()

    f = c.addFunction('std::string','getTypeName',const=True,virtual=True)
    f.addImplementation('return "'+name+'";\n')

    sfimp = ''

    for f in fields:
        ff = c.addFunction('gz4d::x3d::'+f['type']+' &',f['name'])
        ff.addImplementation('return _'+f['name']+';\n')

        sfimp += 'if(f == "'+f['name']+'")\n'
        sfimp += '{\n'
        sfimp += '    _'+f['name']+'=v;\n'
        sfimp += '    return;\n'
        sfimp += '}\n'

        if f.has_key('default'):
            con.addInitializer('_'+f['name']+'("'+'\\"'.join(f['default'].split('"'))+'")')

    

    #for f in node.getchildren():
        #if f.tag == 'Field':
            #fname = f.attrib['name']
            #if fname != '':
                #ftype = f.attrib['type']

                #ff = c.addFunction('gz4d::x3d::'+ftype+' &',fname)
                #ff.addImplementation('return _'+fname+';\n')

                #sfimp += 'if(f == "'+fname+'")\n'
                #sfimp += '{\n'
                #sfimp += '    _'+fname+'=v;\n'
                #sfimp += '    return;\n'
                #sfimp += '}\n'

                #if f.attrib.has_key('default'):
                    #default = f.attrib['default']
                    #con.addInitializer('_'+fname+'("'+'\\"'.join(default.split('"'))+'")')


    #f = c.addFunction('void','addNode')
    #f.addParameter('sai::Node','n')

    if name == 'X3DNode':
        f = c.addFunction('gz4d::x3d::SFString &','containerField',virtual=True)
        f.addImplementation('return _containerField;\n')
        con.addInitializer('_containerField("children")')

    f = c.addFunction('void','set_field',virtual=True)
    f.addParameter('std::string const &','f')
    f.addParameter('std::string const &','v')
    if b is not None:
        sfimp += b+'::set_field(f,v);\n'
    f.addImplementation(sfimp)

    an = c.addFunction('void','append_node',virtual=True)
    an.addParameter('SFNode const &','n')
    animp = 'boost::shared_ptr<X3DNode> np = n;\n'
    for f in fields:
        if f['type'] in ('SFNode','MFNode'):
            animp += 'if (std::string(np->containerField()) == "'+f['name']+'")\n'
            animp += '{\n'
            if f['type'] == 'SFNode':
                animp += '    _'+f['name']+' = n;\n'
            else:
                animp += '    _'+f['name']+'.append(np);\n'
            animp += '    return;\n'
            animp += '}\n'
    if b is not None:
        animp += b+'::append_node(n);\n'
    an.addImplementation(animp)

    pf = c.addFunction('std::string','getString',virtual=True)
    pf.addParameter('std::string const &','indent')
    pfimp  = 'std::stringstream ret;\n'
    pfimp += 'ret << indent << "'+name+'";\n'
    for f in fields:
        pfimp += 'ret << "\\n" << indent << "\\t'+f['name']+'\\t";\n'
        pfimp += 'ret << _'+f['name']+'.getString(indent+"\\t");\n'
    if b is not None:
        pfimp += 'ret << "\\n" << '+b+'::getString(indent+".\\t");\n'

    pfimp += 'return ret.str();\n'
    pf.addImplementation(pfimp)

def ResolveType(field,root):
    ft = field.get('type')
    while not ft[:2] in ('SF','MF'):
        if ft.startswith('xs:'):
            ft = 'SFString'
        else:
            for t in root.findall('Type'):
                if t.get('name') == ft:
                    ft = t.find('Base').get('type')
                    break
    
    field.set('type',ft)

    

def GenerateLoader(parent,indent,root):
    c = Class(parent,indent,'loader')
    c.setNamespace('gz4d::x3d')
    c.setIncludePrefix('gz4d/x3d/ng')

    c.addDeclarationHeader('gz4d/gz4d.h')
    c.addDeclarationHeader('gz4d/util/Filename.h')
    c.addDeclarationHeader('gz4d/util/Xml.h')
    c.addDeclarationHeader('boost/shared_ptr.hpp')
    c.addDeclarationHeader('map')
    c.addDeclarationHeader('gz4d/x3d/ng/SFNode.h')

    c.addForwardDeclaration('class Scene;')

    c.addImplementationHeader('gz4d/x3d/ng/Scene.h')

    c.setScope('public')
    con = c.addConstructor()
    cimpl  = 'if (elementLoaders.empty())\n'
    cimpl += '{\n'

    lf = c.addFunction('boost::shared_ptr<gz4d::x3d::Scene>','load')
    lf.addParameter('gz4d::Filename const &','fname')

    imp  = 'boost::shared_ptr<Scene> ret(new Scene);\n'
    imp += 'xml::Doc doc;\n'
    imp += 'doc.ParseFile(fname);\n'
    imp += 'xml::Node x3d = doc.GetFirstNode("X3D");\n'
    imp += '\n'
    imp += 'if (!x3d)\n'
    imp += '    return boost::shared_ptr<Scene>();\n'
    imp += '\n'
    imp += 'xml::Node sn = x3d.FirstNode("Scene");\n'
    imp += 'if (!sn)\n'
    imp += '    return boost::shared_ptr<Scene>();\n'
    imp += '\n'
    imp += 'for (xml::Node se = sn.GetFirstNode(); se; ++se)\n'
    imp += '{\n'
    imp += '    if (elementLoaders.find(se.Name()) != elementLoaders.end())\n'
    imp += '    {\n'
    imp += '        SFNode n = elementLoaders[se.Name()](se);\n'
    imp += '        ret->children().append(n);\n'
    imp += '    }\n'
    imp += '}\n'
    imp += 'return ret;\n'
    lf.addImplementation(imp)

    c.setScope('private')

    c.addTypedef('gz4d::x3d::SFNode (*ElementLoaderPtr)(xml::Node const &)','')
    c.addVariable('std::map<std::string,gz4d::x3d::loader::ElementLoaderPtr>','elementLoaders',static=True)

    for n in root.findall('Node'):
        if not n.attrib.has_key('abstract'):
            b = n.find('Base')
            while b is not None and b.attrib['type'] != 'X3DNode':
                for bn in root.findall('Node'):
                    if bn.attrib['name'] == b.attrib['type']:
                        b = bn.find('Base')
                        break

            if b is not None:
                c.addImplementationHeader('gz4d/x3d/ng/'+n.attrib['name']+'.h')

                lef = c.addFunction('gz4d::x3d::SFNode','load'+n.attrib['name'],static=True)
                lef.addParameter('xml::Node const &','node')

                imp  = 'SFNode ret;\n'
                imp += 'boost::shared_ptr<gz4d::x3d::X3DNode> n(new '+n.attrib['name']+');\n'
                imp += 'for (xml::Attribute a = node.GetFirstAttribute(); a; ++a)\n'
                imp += '    n->set_field(a.Name(),a.Value());\n'
                imp += 'for (xml::Node e = node.GetFirstNode(); e; ++e)\n'
                imp += '    if (elementLoaders.find(e.Name()) != elementLoaders.end())\n'
                imp += '    {\n'
                imp += '        SFNode sn = elementLoaders[e.Name()](e);\n'
                imp += '        n->append_node(sn);\n'
                imp += '    }\n'
                imp += 'ret = n;\n'
                imp += 'return ret;\n'
                lef.addImplementation(imp)

                cimpl += '    elementLoaders["'+n.attrib['name']+'"] = &load'+n.attrib['name']+';\n'

    cimpl += '}\n'
    con.addImplementation(cimpl)

def usage():
    print 'usage: x3d_html2cxx_xml.py [key=value ...]'
    print '     keys: hdir,cdir,in=x3d_xsd.xml,out=x3d_cxx.xml'
    sys.exit(1)


if __name__ == '__main__':
    params = {'in':'x3d_xsd.xml','out':'x3d_cxx.xml','hdir':'../include/gz4d/x3d/ng','cdir':'../src/x3d/ng'}

    for a in sys.argv[1:]:
        if '=' in a:
            k,v = a.split('=',1)
            params[k] = v
        else:
            usage()

    out_root = xml.etree.ElementTree.Element('CXX')

    indent = '\n  '
    out_root.text = indent
    out_root.tail = '\n'

    out_root.attrib['hdir'] = params['hdir']
    out_root.attrib['cdir'] = params['cdir']


    tree = xml.etree.ElementTree.parse(params['in'])
    root = tree.getroot()

    GenerateFields(out_root,indent,root)

    agroups = root.findall('AttributeGroup')

    for ag in agroups:
        for f in ag.findall('Field'):
            t = f.get('type')
            if not t[:2] in ('SF','MF'):
                ResolveType(f,root)

    for c in root.findall('Node'):
        for f in c.findall('Field'):
            t = f.get('type')
            if not t[:2] in ('SF','MF'):
                #print t,
                ResolveType(f,root)
                #print f.get('type')
        GenerateNode(c,out_root,indent,agroups)

    GenerateLoader(out_root,indent,root)

    out = xml.etree.ElementTree.ElementTree(out_root)
    out.write(params['out'])

