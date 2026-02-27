#!/usr/bin/env python

import xml.dom.minidom
import xml.xpath

doc = xml.dom.minidom.parse('x3d-3.0.xsd')

for n in doc.childNodes:
    if n.nodeType == n.ELEMENT_NODE:
        ns = n.attributes['xmlns:xs'].value
        break

def xeval(query,node):
    c = xml.xpath.Context.Context(node, processorNss={'xs':ns})
    return xml.xpath.Evaluate(query,node,c)

def readAttributes(node):
    ret = {}
    for a in xeval('.//xs:attribute',node):
        attrib = {}
        name = xeval('string(@name)',a)
        t = xeval('string(@type)',a)
        if not t:
            t = xeval('string(xs:simpleType/xs:restriction/@base)',a)
        attrib['type']=t
        if len(xeval('@default',a)) > 0:
            attrib['default'] = xeval('string(@default)',a)
        ret[name] = attrib
        ret[name]['array'] = (xeval('string(ancestor-or-self::*/@maxOccurs)',a) == 'unbounded')
    for a in xeval('.//xs:group',node):
        ref = xeval('string(@ref)',a)
        if ref:
            ret[ref] = {'type':'groupRef'}
            ret[ref]['array'] = (xeval('string(ancestor-or-self::*/@maxOccurs)',a) == 'unbounded')
    for a in xeval('.//xs:element',node):
        ref = xeval('string(@ref)',a)
        if ref:
            ret[ref] = {'type':'elementRef'}
            ret[ref]['array'] = (xeval('string(ancestor-or-self::*/@maxOccurs)',a) == 'unbounded')
    return ret

types = {}

for e in xeval('/xs:schema/xs:simpleType',doc):
    name = xeval('string(@name)',e)
    types[name] = {}
    base = xeval('string(xs:restriction/@base)',e)
    if base:
        types[name]['base'] = base
    listType = xeval('string(xs:list/@itemType)',e)
    if listType:
        types[name]['array'] = listType
    types[name]['attributes']={}

for e in xeval('/xs:schema/xs:complexType',doc):
    name = xeval('string(@name)',e)
    types[name] = {}
    base = xeval('string(xs:complexContent/xs:extension/@base)',e)
    if base:
        types[name]['base'] = base
    types[name]['attributes'] = readAttributes(e)

elements = {}

for e in xeval('/xs:schema/xs:element',doc):
    name = xeval('string(@name)',e)
    elements[name] = {}
    elements[name]['base'] = xeval('string(xs:complexType/xs:complexContent/xs:extension/@base)',e)
    elements[name]['attributes'] = readAttributes(e)

groups = {}

for g in xeval('/xs:schema/xs:group',doc):
    name = xeval('string(@name)',g)
    groups[name] = {}
    groups[name]['attributes'] = readAttributes(g)


if __name__ == "__main__":
    for g,h in ((types,'TYPES'),(groups,'GROUPS'),(elements,'ELEMENTS')):
        print '****',h,'****'
        sn = list(g.keys())
        sn.sort()
        for n in sn:
            e = g[n]
            print n,
            if 'base' in e:
                print '(',e['base'],')'
            else:
                print
            san = list(e['attributes'].keys())
            san.sort()
            for an in san:
                a = e['attributes'][an]
                print ' ',a['type'],an,
                if a['array']:
                    print '[]',
                if 'default' in a:
                    print '=',a['default']
                else:
                    print
