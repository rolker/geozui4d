#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

# coding: utf-8

import xml.etree.ElementTree
import sys


class Node:
    def __init__(self,e,ns,index,appendIndex = False):
        self.index = index
        self.type = e.tag[len(ns):]
        if appendIndex:
            if len(index):
                appendIndex = False
            try:
                index[e.attrib['name']] = self
            except KeyError:
                pass

        self.children = []
        for c in e.getchildren():
            self.children.append(Node(c,ns,index,appendIndex))

        self.attrib = e.attrib
        self.text = e.text

    def Get(self,childType):
        for c in self.children:
            if c.type == childType:
                return c

class Schema(Node):
    def __init__(self,fname):
        tree = xml.etree.ElementTree.parse(fname)
        root = tree.getroot()
        ns =  root.tag.split('}',1)[0]+'}'
        name = root.tag.split('}',1)[1]
        if name == 'schema':
            Node.__init__(self,root,ns,{},True)


if __name__ == '__main__':
    for a in sys.argv[1:]:
        s = Schema(a)
        print a,len(s.index)

        types = {}

        for n in s.children:
            if not types.has_key(n.type):
                types[n.type] = []
            if n.attrib.has_key('name'):
                types[n.type].append(n.attrib['name'])

        for t,elements in types.iteritems():
            print t
            for e in elements:
                print '\t'+e
                