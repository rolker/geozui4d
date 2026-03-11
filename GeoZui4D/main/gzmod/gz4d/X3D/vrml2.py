#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import x3d

def getAttrib(name,attrib,attribDict,arrayFlag):
    if attrib['type'] == 'xs:NMTOKEN':
        return
    if attrib['type'] == 'groupRef':
        getElement(x3d.groups[name],attribDict,arrayFlag or attrib['array'])
        return
    if attrib['type'] == 'elementRef':
        e = x3d.elements[name]
        while e is not None:
            if 'containerField' in e['attributes']:
                if attrib['array'] or arrayFlag:
                    attribDict[e['attributes']['containerField']['default']]=('MFNode',None)
                else:
                    attribDict[e['attributes']['containerField']['default']]=('SFNode',None)
            if 'base' in e:
                e = x3d.types[e['base']]
            else:
                e = None
        return
    
    if 'default' in attrib:
        attribDict[name]=(attrib['type'],attrib['default'])
    else:
        attribDict[name]=(attrib['type'],None)

def getElement(element,attribDict,arrayFlag=False):
    while element is not None:
        for n,a in element['attributes'].iteritems():
            getAttrib(n,a,attribDict,arrayFlag)
        try:
            b = element['base']
            element = x3d.types[b]
        except KeyError:
            element = None

#create types dict

#non float types
types={
'Bool':{'type':'bool','count':1},
'Int32':{'type':'int','count':1},
'String':{'type':'string','count':1},
'Image':{'type':'image','count':1},
'Node':{'type':'node','count':1},
}

#float types
c = 1
for i in (('Float','Double','Time'),('Vec2f','Vec2d'),('Vec3f','Vec3d','Color'),('Rotation','ColorRGBA')):
    for j in i:
        types[j] = {'type':'float','count':c}
    c += 1


# nodes

nodes={}

for l,e in x3d.elements.iteritems():
    nodes[l] = {}
    getElement(e,nodes[l])

if __name__ == "__main__":
    print '#!/usr/bin/env/ python\n\n'
    tl = []
    sk = types.keys()
    sk.sort()
    for l in sk:
        tl.append(''.join(("'",l,"':",str(types[l]))))
    print ''.join(('types={\n',',\n'.join(tl),'}\n'))

    nl = []
    sk = nodes.keys()
    sk.sort()
    for n in sk:
        e = nodes[n]
        snl = []
        snk = e.keys()
        snk.sort()
        for sn in snk:
            sub = e[sn]
            snl.append(''.join(("'",sn,"':",str(sub))))
        nl.append(''.join(("'",n,"':{\n  ",',\n  '.join(snl),'}')))
    print ''.join(('nodes={\n',',\n'.join(nl),'\n}\n'))
