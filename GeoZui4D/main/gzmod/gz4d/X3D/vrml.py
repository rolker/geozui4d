#!/usr/bin/env python

import vrml2_static as vrml2

# nodeDescriptions = {
# 'Appearance':{
#     'material':('node',None),
#     'texture':('node',None),
#     'textureTransform':('node',None)},
# 'IndexedFaceSet':{
#     'color':('node',None),
#     'coord':('node',None),
#     'normal':('node',None),
#     'texCoord':('node',None),
#     'ccw':('b',True),
#     'colorIndex':('ia',None),
#     'colorPerVertex':('b',True),
#     'convex':('b',True),
#     'coordIndex':('ia',None),
#     'creaseAngle':('f',0),
#     'normalIndex':('ia',None),
#     'normalPerVertex':('b',True),
#     'solid':('b',True),
#     'texCoordIndex':('ia',None)},
# 'Material':{
#     'ambientIntensity':('f',0.2),
#     'diffuseColor':('3f',(0.8,0.8,0.8)),
#     'emissiveColor':('3f',(0,0,0)),
#     'shineness':('f',0.2),
#     'specularColor':('3f',(0,0,0)),
#     'transparency':('f',0)},
# 'Shape':{
#     'appearance':('node',None),
#     'geometry':('node',None)},
# 'TimeSensor':{
#     'cycleInterval':('f',1),
#     'enabled':('b',True),
#     'loop':('b',False),
#     'startTime':('f',0),
#     'endTime':('f',0)},
# 'Transform':{
#     'center':('3f',(0,0,0)),
#     'children':('nodeArray',None),
#     'rotation':('4f',(0,0,1,0)),
#     'scale':('3f',(1,1,1)),
#     'scaleOrientation':('4f',(0,0,1,0)),
#     'translation':('3f',(0,0,0)),
#     'bboxCenter':('3f',(0,0,0)),
#     'bboxSize':('3f',(-1,-1,-1))}
# }

class Node:
    def __init__(self, scene, nodeType):
        self.scene = scene
        self.attributes = {}
        self.nodeType = nodeType
        self.name = None
        try:
            scene.nodeCounts[nodeType] += 1
        except KeyError:
            scene.nodeCounts[nodeType] = 1

    def parseNode(self, source, part):
        name = None
        while part == 'ROUTE':
            source.next()
            source.next()
            source.next()
            part = source.next()
        if part == 'DEF':
            name = source.next()
            #print 'Name:',name
            part = source.next()
        if part == 'USE':
            ref = source.next()
            rnode = self.scene.nodeNames[ref]
            node = Node(self.scene,rnode.nodeType)
            node.attributes['USE'] = ref
            return node
        if part in vrml2.nodes:
            #print 'Node:',part
            node = Node(self.scene,part)
            node.parse(source)
            if name is not None:
                self.scene.nodeNames[name] = node
                node.name = name
            return node
        else:
            print '*** unknown node',part
            self.SkipNode(source)

    def parseValue(self, source, t, d):
        if t in vrml2.types:
            te = vrml2.types[t]
            if te['type'] == 'bool':
                return(d == 'TRUE')
            if te['type'] == 'float':
                if te['count'] > 1:
                    ret = [float(d)]
                    for i in range(te['count']-1):
                        ret.append(float(source.next()))
                    return ret
                else:
                    return float(d)
            if te['type'] == 'int':
                return int(d)
            if te['type'] == 'string':
                return d
            if te['type'] == 'image':
                img = {}
                img['width'] = int(d)
                img['height'] = int(source.next())
                img['components'] = int(source.next())
                img['image'] = []
                for j in range(img['width']*img['height']):
                    img['image'].append(source.next())
                return img
            if te['type'] == 'node':
                return self.parseNode(source,d)

    def parse(self,source):
        #if self.nodeType == 'USE':
            #self.ref = source.next()
            #return
        if self.nodeType == 'Scene':
            self.attributes['children'] = []
            for part in source:
                self.attributes['children'].append(self.parseNode(source,part))
        else:
            if source.next() != '{':
                return
            attribs = vrml2.nodes[self.nodeType]
            for part in source:
                while part == 'ROUTE':
                    source.next()
                    source.next()
                    source.next()
                    part = source.next()
                data = None
                if part == '}':
                    break
                t = attribs[part][0]
                data = None
                if t.startswith('SF'):
                    data = self.parseValue(source,t[2:],source.next())
                if t.startswith('MF'):
                    data = []
                    subpart = source.next()
                    if subpart != '[':
                        data.append(self.parseValue(source,t[2:],subpart))
                    else:
                        for i in source:
                            if i == ']':
                                break
                            data.append(self.parseValue(source,t[2:],i))
                self.attributes[part] = data

    def SkipNode(self, source):
        if source.next() == '{':
            depth = 0
            while depth >= 0:
                c = source.next()
                if c == '{':
                    depth += 1
                if c == '}':
                    depth -= 1

    def PrettyString(self,indent=0,showValues = False):
        if showValues:
            prefix = '  '*(indent+1)
            lineEnd = ' {\n'
        else:
            prefix = ' '
            lineEnd = ' { '
        ret = ''.join(('  '*indent,self.nodeType,lineEnd))
        #print 'Node::PrettyString',self.nodeType
        for k,a in self.attributes.iteritems():
            #print ' - ',k,a
            if k == 'USE':
                ret = ''.join((ret,prefix,'USE:',a))
            else:
                t = vrml2.nodes[self.nodeType][k][0]
                if not t in ('SFNode','MFNode'):
                    ret = ''.join((ret,prefix,k,':'))
                    if t.startswith('MF'):
                        ret = ''.join((ret,' [ '))
                        if showValues:
                            for v in a:
                                ret = ' '.join((ret,self.StringValue(indent+2,t,v,showValues)))
                            ret = ''.join((ret,' ]\n'))
                        else:
                            ret = ''.join((ret,str(len(a)),' x ',t,' ] '))
                    else:
                        if showValues:
                            ret = ''.join((ret,self.StringValue(indent+2,t,a,showValues)))

        for k,a in self.attributes.iteritems():
            if k != 'USE':
                t = vrml2.nodes[self.nodeType][k][0]
                if t in ('SFNode','MFNode'):
                    ret = ''.join((ret,prefix,k,':'))
                    if t.startswith('MF'):
                        ret = ''.join((ret,' [ '))
                        for v in a:
                            ret = ''.join((ret,self.StringValue(indent+2,t,v,showValues)))
                        ret = ''.join((ret,'  '*(indent+1),']\n'))
                    else:
                        ret = ''.join((ret,self.StringValue(indent+2,t,a,showValues)))

        if showValues:
            return ''.join((ret,'  '*indent,'}\n'))
        else:
            return ''.join((ret,' }\n'))

    def StringValue(self,indent,t,v,showValues):
        td = vrml2.types[t[2:]]
        if td['type'] == 'image':
            return ' '.join((v[0],'x',v[1],'(',v[2],'components )'))
        if td['type'] == 'node':
            return ''.join(('\n',v.PrettyString(indent,showValues)))
        return str(v)

    def x3dValue(self,t,v):
        td = vrml2.types[t[2:]]
        if td['count'] > 1:
            ret = str(v[0])
            for i in v[1:]:
                ret = ' '.join((ret,str(i)))
            return ret
        else:
            if td['type'] == 'bool':
                if v:
                    return 'true'
                else:
                    return 'false'
            #if td['type'] == 'string':
                #return v.strip('"')
            return str(v)

    def x3dString(self, indent=0):
        #print ' '*indent,'x3dString:',self.nodeType,
        ret = ''.join(('  '*indent,'<',self.nodeType))
        if self.name is not None:
            ret = ''.join((ret,' DEF="',self.name,'"'))
        for k,a in self.attributes.iteritems():
            #print k,
            if k == 'USE':
                ret = ''.join((ret,' USE="',a,'"'))
            else:
                t = vrml2.nodes[self.nodeType][k][0]
                if not t in ('SFNode','MFNode'):
                    if t == 'MFString':
                        ret = ''.join((ret,' ',k,"='"))
                    else:
                        if t == 'SFString':
                            ret = ''.join((ret,' ',k,'='))
                        else:
                          ret = ''.join((ret,' ',k,'="'))
                    if t.startswith('MF'):
                        if len(a) > 0:
                            ret = ''.join((ret,self.x3dValue(t,a[0])))
                            for v in a[1:]:
                                ret = ' '.join((ret,self.x3dValue(t,v)))
                    else:
                        ret = ''.join((ret,self.x3dValue(t,a)))
                    if t == 'MFString':
                        ret = ''.join((ret,"'"))
                    else:
                        if t != 'SFString':
                            ret = ''.join((ret,'"'))
        ret = ''.join((ret,'>\n'))
        #print

        for k,a in self.attributes.iteritems():
            if k != 'USE':
                t = vrml2.nodes[self.nodeType][k][0]
                if t in ('SFNode','MFNode'):
                    #print t
                    if t.startswith('MF'):
                        for v in a:
                            ret = ''.join((ret,v.x3dString(indent=indent+1)))
                    else:
                        ret = ''.join((ret,a.x3dString(indent=indent+1)))
        ret = ''.join((ret,'  '*indent,'</',self.nodeType,'>\n'))
        return ret

class Scene:
    def __init__(self):
        self.nodeCounts = {}
        self.root = Node(self,'Scene')
        self.source = None
        self.nodeNames = {}

    def nextPart(self):
        if self.source is None:
            return
        next = None
        for l in self.source:
            line = l.split('#',1)[0]
            #print 'nextPart: line:',line
            parts = [line]
            # find strings
            foundAllStrings = False
            while not foundAllStrings:
                lineparts = parts[-1].split('"',2)
                if len(lineparts) == 3:
                    parts.pop()
                    if lineparts[0] != '':
                        parts.append(lineparts[0])
                    parts.append(lineparts[1].join(('"','"')))
                    if lineparts[2] != '':
                        parts.append(lineparts[2])
                    else:
                        foundAllStrings = True
                else:
                    foundAllStrings = True
            newparts = []
            for p in parts:
                if len(p) > 0 and p[0] == '"':
                    newparts.append(p)
                else:
                    for sp in p.split():
                        sp = sp.strip()
                        sp = sp.strip(',')
                        if len(sp) > 1 and sp[0] in ('[','{'):
                            newparts.append(sp[0])
                            sp = sp[1:]
                        if len(sp) > 1 and sp[-1] in (']','}'):
                            newparts.append(sp[:-1])
                            newparts.append(sp[-1])
                        else:
                            newparts.append(sp)
            #print 'nextPart: parts:',newparts
            for p in newparts:
                yield p


    def parse(self, source):

        # header

        head = source.readline().split()
        #print 'head:',head
        if len(head) < 2:
            return
        if head[0] != '#VRML':
            return
        self.version = head[1]
        #print self.version

        self.source = source

        self.root.parse(self.nextPart())

    def PrettyString(self, showValues = False):
        return ''.join(('VRML version ',self.version,'\n',self.root.PrettyString(showValues=showValues)))
    
    def x3dString(self):
        rootnodes = self.root.x3dString(indent = 1)
        return rootnodes.join(('<?xml version="1.0" encoding="UTF-8"?>\n<X3D profile="Immersive" version="3.0">\n','</X3D>'))


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print "Usage: vrml.py file.wrl"
    else:
        infile = open(sys.argv[1])

        s = Scene()
        s.parse(infile)
        s.PrettyPrint()
