#!/usr/bin/env python
# coding: utf-8

import xml.etree.ElementTree
import sys
import os
import HTMLParser

class ParseError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value


class Field:
    def __init__(self,line,parent):
        if not '[' in line:
            raise ParseError(line)

        props = {}

        t,rest = line.split('[',1)
        t = t.strip()

        if t == 'SFBoolean':
            t = 'SFBool'

        #if not t.startswith('X3D') and not t.startswith('SF') and not t.startswith('MF'):
            #raise ParseError(line)

        #if '<' in t or '/' in t or t == 'fieldType':
            #raise ParseError(line)

        props['type']=t

        access,rest = rest.split(']',1)

        props['input'] = str('in' in access).lower()
        props['output'] = str('out' in access).lower()


        parts = rest.split(None,1)
        if len(parts) == 0:
            raise ParseError(line)
        props['name'] = parts[0]

        if len(parts) == 2:
            rest = parts[1]
        else:
            rest = None

        if rest is not None:

            if len(rest):
                if rest[0] == '[':
                    d,r = rest[1:].split(']',1)
                    if len(d):
                        props['default'] = d
                    if len(r.strip()):
                        props['restriction'] = r.strip()
                else:
                    d1 = None
                    r1 = None
                    d2 = None
                    r2 = None

                    if '[' in rest:
                        d1,r1 = rest.split('[',1)
                        r1 = '['+r1

                    if '(' in rest:
                        d2,r2 = rest.split('(',1)
                        r2 = '('+r2

                    if d1 is None and d2 is None:
                        props['default'] = rest
                    else:
                        if d1 is not None and d2 is not None:
                            if len(d1) < len(d2):
                                if len(d1.strip()):
                                    props['default'] = d1.strip()
                                props['restriction'] = r1
                            else:
                                if len(d2.strip()):
                                    props['default'] = d2.strip()
                                props['restriction'] = r2
                        else:
                            if d2 is None:
                                if len(d1.strip()):
                                    props['default'] = d1.strip()
                                props['restriction'] = r1
                            else:
                                if len(d2.strip()):
                                    props['default'] = d2.strip()
                                props['restriction'] = r2

        self.element = xml.etree.ElementTree.SubElement(parent,'Field',props)
        #self.element.text = parent.text+'  '
        self.element.tail = parent.text



class Node:
    def __init__(self, lines, parent):

        l1 = None
        f = None

        line_array = []
        for l in lines.splitlines():
            if len(line_array) and len(line_array[-1]) and line_array[-1][-1] == ',':
                line_array[-1] += l.strip()
                print 'fixed:',line_array[-1]
            else:
                line_array.append(l)
        for l in line_array:
            #print l
            if l1 is None:
                if '{' in l or 'TextureProperties' in l:
                    l1 = l

                    self.element = xml.etree.ElementTree.SubElement(parent,'Node')
                    self.element.text = parent.text + '  '
                    self.element.tail = parent.text

                    if ':' in l1:
                        name,bases = l1.split(':',1)
                        bases = bases.split('{')[0].strip().split(',')
                        for b in bases:
                            bs = b.strip()
                            if bs == 'X3DURLObject':
                                bs = 'X3DUrlObject'
                            be = xml.etree.ElementTree.SubElement(self.element,'Base',{'type':bs})
                            be.tail = self.element.text

                    else:
                        name = l1.split('{')[0]

                    self.element.attrib['name'] =  name.strip()
            else:
                if '}' in l:
                    break
                else:
                    try:
                        Field(l.strip(),self.element)
                    except ParseError, e:
                        print name.strip(),'- failed to parse field:',e

        if l1 is None:
            raise ParseError(''.join(lines))





class X3DParser(HTMLParser.HTMLParser):
    def __init__(self,root):
        HTMLParser.HTMLParser.__init__(self)
        self.node = None
        self.root = root

    def handle_starttag(self, tag, attrs):
        if tag == "pre":
            if len(attrs):
                for a in attrs:
                    if a[0] == 'class' and a[1] == 'node':
                        self.node = ""
            else:
                self.node = ""

    def handle_data(self, data):
        if self.node is not None:
            self.node += data

    def handle_endtag(self, tag):
        if self.node is not None and tag == "pre":
            #print self.node
            try:
                Node(self.node,self.root)
            except ParseError, e:
                print 'failed to parse element:'
                print e
            self.node = None

    def handle_charref(self,name):
        if self.node is not None:
            n = None
            if name == "8734":
                n = "inf"
            if name == "960":
                n = "pi"

            if n is not None:
                self.node += n
            else:
                self.node += 'unknown#:' + name

    def handle_entityref(self,name):
        if self.node is not None:
            n = None
            if name == "pi":
                n = "pi"
            if name == "quot":
                n = '"'

            if name == "lt":
                n = '<'

            if name == "gt":
                n = '>'

            if n is not None:
                self.node += n
            else:
                self.node += 'unknown#:' + name


class X3D:
    def __init__(self,fname):
        self.root = xml.etree.ElementTree.Element('X3D')
        self.root.text = '\n  '
        self.root.tail = '\n'

        for fn in open(fname):
            component = os.path.splitext(os.path.basename(fn))[0]
            print '***',component,'***'

            p = X3DParser(self.root)
            try:
                p.feed(open(fn.strip()).read())
            except HTMLParser.HTMLParseError, e:
                print e


def usage():
    print 'usage: x3d_html2xml.py [key=value ...]'
    sys.exit(1)

if __name__ == '__main__':
    params = {}

    for a in sys.argv[1:]:
        if '=' in a:
            k,v = a.split('=',1)
            params[k] = v
        else:
            usage()


    x3d = X3D('x3d_html_components.txt')
    out = xml.etree.ElementTree.ElementTree(x3d.root)
    out.write(params['out'])

