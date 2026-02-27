#!/usr/bin/env python
# -*- coding: utf-8 -*-
# coding: utf-8

import xml.etree.ElementTree
import sys
import os
import codecs

class FileUpdater:
    def __init__(self,path):
        self.path = path
        self.content = ''

    def __del__(self):
        if os.path.exists(self.path) and codecs.open(self.path,encoding='utf-8').read() == self.content:
            return
        codecs.open(self.path,'w',encoding='utf-8').write(self.content)

    def write(self,s):
        self.content += s


class CodeGenerator:
    def __init__(self,name,namespace,hprefix,hdir,cdir,hincs,cincs=None):
        self.name = name
        self.namespace = namespace
        self.hprefix = hprefix
        self.indent = ''

        #self.hout = open(os.path.join(hdir,name+'.h'),'w')
        #self.cout = open(os.path.join(cdir,name+'.cpp'),'w')
        self.hout = FileUpdater(os.path.join(hdir,name+'.h'))
        self.cout = FileUpdater(os.path.join(cdir,name+'.cpp'))

        self.hincludes = hincs
        self.cincludes = cincs

        self.initHeader()
        self.openNamespace()

        self.initImplementation()

    def __del__(self):
        self.closeNamespace()


    def initHeader(self):

        guard = []
        if self.hprefix is not None:
            for pi in self.hprefix.split('/'):
                guard.append(pi.upper())
        guard.append(self.name.upper())
        guard.append('H')
        guard = '_'.join(guard)

        self.hout.write('#ifndef '+guard+'\n')
        self.hout.write('#define '+guard+'\n')
        self.hout.write('\n')
        for i in self.hincludes:
            self.hout.write('#include<'+i+'>\n')
        self.hout.write('\n')

    def openNamespace(self):
        if self.namespace is not None:
            for n in self.namespace.split('::'):
                self.hout.write(self.indent+'namespace '+n+'\n')
                self.hout.write(self.indent + '{\n')
                self.indent += '    '


    def closeNamespace(self):
        while len(self.indent):
            self.indent = self.indent[:-4]
            self.hout.write(self.indent+'}\n')
        self.hout.write('\n')
        self.hout.write('#endif\n')
        self.hout.write('\n')


    def initImplementation(self):
        self.cout.write('#define GZ4D_INTERNAL\n')
        if self.hprefix is not None:
            self.cout.write('#include <'+os.path.join(self.hprefix,self.name)+'.h>\n')
        else:
            self.cout.write('#include "'+self.name+'.h"\n')
        if self.cincludes is not None:
            for i in self.cincludes:
                self.cout.write('#include<'+i+'>\n')

        self.cout.write('\n')

    def hwrite(self,line):
        self.hout.write(line)

    def cwrite(self,line):
        self.cout.write(line)




def GenerateClass(node,hdir,cdir):

    name = node.attrib['name']
    namespace = node.get('namespace')
    inc_prefix = node.get('include_prefix')

    base = None
    if node.attrib.has_key('bases'):
        base = node.attrib['bases']

    hincs = []
    dec = node.find('Declaration')
    if dec is not None:
        for h in dec.findall('Header'):
            hincs.append(h.text)

    cg = CodeGenerator(name,namespace,inc_prefix,hdir,cdir,hincs)

    if dec is not None:
        for f in dec.findall('ForwardDeclaration'):
            cg.hwrite(cg.indent+f.text+'\n')


    imp = node.find('Implementation')
    if imp is not None:
        for h in imp.findall('Header'):
            cg.cwrite('#include<'+h.text+'>\n')
    cg.cwrite('\n')


    docs = node.find('Documentation')
    d = ''
    if docs is not None:
        dl = []
        if docs.text is not None:
            dl += docs.text.split('\n')
        if docs.attrib.has_key('source'):
            dl.append(docs.attrib['source'])
        if len(dl):
            d += '        /// ' +  '\n        /// '.join(dl) + '\n'

    d += '        class GZ4DEXPORT '+name
    if base is not None:
        d += ': public ' + base + '\n'
    else:
        d += '\n'
    cg.hwrite(d)
    cg.hwrite('        {\n')

    indent1 = '            '
    indent2 = '                '
    indent = indent1
    
    def writeParts(n,indent):
        for c in n.getchildren():
            docs = c.find('Documentation')
            d = []
            if docs is not None:
                if docs.text is not None:
                    d += docs.text.split('\n')
                    if docs.attrib.has_key('source'):
                        d.append(docs.attrib['source'])
            if len(d):
                cg.hwrite(indent+'/// '+('\n'+indent+'/// ').join(d)+'\n')

            if c.tag in ('private','protected','public'):
                cg.hwrite(indent1+c.tag+':\n')
                writeParts(c,indent2)
            if c.tag == 'Typedef':
                cg.hwrite(indent+'typedef '+c.attrib['type']+' '+c.attrib['name']+';\n')

            if c.tag == 'Variable':
                t = c.attrib['type'] + ' '
                n = c.attrib['name']

                s = ''
                if c.get('static') == 'true':
                    s = 'static '
                    if namespace is None:
                        ns = ''
                    else:
                        ns = namespace+'::'

                    cg.cwrite(t+ns+name+'::'+n+';\n\n')
                cg.hwrite(indent+s+t+n+';\n')

                i = c.find('Implementation')
                if i is not None and i.text is not None:
                    cg.cwrite('\n')
                    cg.cwrite(i.text)
                    cg.cwrite('\n')

            if c.tag == 'Enum':
                cg.hwrite(indent+'enum '+c.attrib['name']+' {\n')
                for v in c.findall('Value'):
                    cg.hwrite(indent+'    '+v.text+',\n')
                cg.hwrite(indent+'};\n')
                cg.hwrite(indent+'\n')

            if c.tag in ('Function','Constructor'):
                s = ''
                if c.get('static') == 'true':
                    s = 'static '
                if c.tag == 'Function':
                    t = c.attrib['type'] + ' '
                    n = c.attrib['name']
                else:
                    t = ''
                    n = name

                if c.attrib.has_key('virtual') and c.attrib['virtual'] == 'true':
                    v = 'virtual '
                else:
                    v = ''

                cg.hwrite(indent+s+v+t+n+'(')
                if namespace is None:
                    ns = ''
                else:
                    ns = namespace+'::'
                cg.cwrite(t+ns+name+'::'+n+'(')

                params = []
                for p in c.findall('Parameter'):
                    params.append(p.attrib['type']+' '+p.attrib['name'])

                cg.hwrite(', '.join(params)+')')
                cg.cwrite(', '.join(params)+')')

                if c.attrib.has_key('const') and c.attrib['const'] =='true':
                    cg.hwrite(' const')
                    cg.cwrite(' const')
                cg.hwrite(';\n')
                cg.cwrite('\n')
                if c.tag == 'Constructor':
                    inits = []
                    for i in c.findall('Initializer'):
                        inits.append(i.text)
                    if len(inits):
                        cg.cwrite('    :'+','.join(inits)+'\n')
                cg.cwrite('{\n')
                i = c.find('Implementation')
                if i is not None and i.text is not None:
                    for l in i.text.splitlines():
                        cg.cwrite('   '+l+'\n')
                cg.cwrite('}\n')
                cg.cwrite('\n')

    writeParts(node,indent1)

    cg.hwrite('        };\n')
    cg.hwrite('\n')





def usage():
    print 'usage: cxx_xml2cxx.py [key=value ...]'
    print '     keys: in=x3d_cxx.xml'
    sys.exit(1)


if __name__ == '__main__':
    params = {'in':'x3d_cxx.xml'}

    for a in sys.argv[1:]:
        if '=' in a:
            k,v = a.split('=',1)
            params[k] = v
        else:
            usage()

    tree = xml.etree.ElementTree.parse(params['in'])
    root = tree.getroot()

    hdir = root.attrib['hdir']
    cdir = root.attrib['cdir']

    for c in root.getchildren():
        if c.tag == 'Class':
            GenerateClass(c,hdir,cdir)

