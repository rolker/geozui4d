#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import iso8211
import tables

decoder = {'RCNM':{10:('DS','Data Set General Information'),
                    20:('DP','Data Set Geographic Reference'),
                    30:('DH','Data Set History'),
                    40:('DA','Data Set Accuracy'),
                    60:('CR','Catalogue Cross Reference'),
                    70:('ID','Data Dictionary Definition'),
                    80:('IO','Data Dictionary Domain'),
                    90:('IS','Data Dictionary Schema'),
                    100:('FE','Feature'),
                    110:('VI','Vector Isolated node'),
                    120:('VC','Vector Connected node'),
                    130:('VE','Vector Edge'),
                    140:('VF','Vector Face')},
            'EXPP':{1:('N','Data set is New'),
                    2:('R','Data set is a revision to an existing one')},
            'PRSP':{1:('ENC','Electronic Navigational Chart'),
                    2:('ODD','IHO Object Catalogue Data Dictionary')},
            'PRIM':{1:('P','Point'),
                    2:('L','Line'),
                    3:('A','Area'),
                    255:('N','Object does not directly reference any spatial objects')},
            'ORNT':{1:('F','Forward'),
                    2:('R','Reverse'),
                    255:('N','NULL')},
            'USAG':{1:('E','Exterior'),
                    2:('I','Interior'),
                    3:('C','Exterior boundary truncated by the data limit'),
                    255:('N','NULL')},
            'MASK':{1:('M','Mask'),
                    2:('S','Show'),
                    255:('N','NULL')},
            'TOPI':{1:('B','Beginning node'),
                    2:('E','End node'),
                    3:('S','Left face'),
                    4:('D','Right face'),
                    5:('F','Containing face'),
                    255:('N','NULL')},
           }

def decode(feature,tag,row=None):
    return decoder[tag][feature.Get(tag,row)]


class S57:

    def __init__(self, fname):
        self.ddf = iso8211.DDF(fname)
        r = self.ddf.nextRecord()
        while r is not None:
            d = r.getData()
            if self.process(d) is not None:
                r = self.ddf.nextRecord()
            else:
                r = None

    def process(self,data):
        if decode(data,'RCNM')[0] == 'DS':
            self.info = DatasetInfo(data)
            print 'info:',self.info
            return self.info

        if decode(data,'RCNM')[0] == 'DP':
            self.geo = DatasetGeoref(data)
            print 'geo:',self.geo
            return self.geo

        if decode(data,'RCNM')[0] in ('VI','VC','VE'):
            return Vector(data)

        if decode(data,'RCNM')[0] == 'FE':
            return Feature(data)

        print decode(data,'RCNM')


class Object:
    def __init__(self, data):
        self.record_name = data.Get('RCNM')
        self.record_id = data.Get('RCID')
        #print decode(data,'RCNM')

    def __str__(self):
        return 'record_name:{0} record_id:{1}'.format(self.record_name,self.record_id)

    def getName(self):
        return (self.record_name << 32)+self.record_id

class DatasetInfo(Object):
    def __init__(self, data):
        Object.__init__(self,data)
        print '  exchange purpose:', decode(data,'EXPP')
        print '  intended usage:', data.Get('INTU')
        print '  dataset name:', data.Get('DSNM')
        print '  product spec:', decode(data,'PRSP')

class DatasetGeoref(Object):
    def __init__(self, data):
        Object.__init__(self,data)
        print '  compilation scale:', data.Get('CSCL')

class Attributes:
    def __init__(self, data):
        self.keys = []
        self.values = {}
        if data is not None:
            for i in range(data.rowCount()):
                k= data.Get('ATTL',i)
                self.keys.append(k)
                self.values[k] = data.Get('ATVL',i)

    def __str__(self):
        if len(self.keys):
            ret = []
            for k in self.keys:
                if tables.attributes[k][2] in ('E','L'):
                    vs = []
                    for vk in self.values[k].split(','):
                        #print k,vk
                        try:
                            vs.append(tables.attributes[k][3][int(vk)])
                        except ValueError:
                            pass
                    v = ','.join(vs)
                else:
                    v = self.values[k]
                ret.append(tables.attributes[k][0]+':'+v)
            return '\n'.join(ret)
        return ''

class Pointers():
    def __init__(self,data,topi=False):
        self.pointers = []
        if data is not None:
            for i in range(data.rowCount()):
                if topi:
                    self.pointers.append((data.Get('NAME',i),decode(data,'ORNT',i),decode(data,'USAG',i),decode(data,'TOPI',i),decode(data,'MASK',i)))
                else:
                    self.pointers.append((data.Get('NAME',i),decode(data,'ORNT',i),decode(data,'USAG',i),decode(data,'MASK',i)))

    def __str__(self):
        if len(self.pointers):
            ret = []
            for p in self.pointers:
                ret.append(str(p))
            return '\n'.join(ret)
        return ''

class Vector(Object):
    def __init__(self, data):
        Object.__init__(self,data)
        print decode(data,'RCNM')

        self.pointers = Pointers(data.Get('VRPT'),True)
        for p in str(self.pointers).splitlines():
            print '  p:',p

        self.attribs = Attributes(data.Get('ATTV'))
        for a in str(self.attribs).splitlines():
            print '  a:',a


class Feature(Object):
    def __init__(self, data):
        Object.__init__(self,data)
        print decode(data,'RCNM')
        print '  primitive:', decode(data,'PRIM')
        #print 'group:',data.Get('GRUP')
        code = data.Get('OBJL')
        print '  label/code:',code,tables.objects[code]
        
        #print 'agent:',data.Get('AGEN')
        self.producing_agency = data.Get('AGEN')
        self.feature_id = data.Get('FIDN')
        self.feature_id_sub = data.Get('FIDS')

        self.attribs = Attributes(data.Get('ATTF'))
        for a in str(self.attribs).splitlines():
            print '  ',a
        self.nattribs = Attributes(data.Get('NATF'))
        for a in str(self.nattribs).splitlines():
            print '  ',a

        print '  ',self.getName(),self.getLongName()
        ffpt = data.Get('FFPT')
        print '  ',ffpt

        fspt = Pointers(data.Get('FSPT'))
        for p in str(fspt).splitlines():
            print '  p:',p


    def getLongName(self):
        return (self.producing_agency << 48)+(self.feature_id<<16)+self.feature_id_sub

if __name__ == '__main__':
    import sys
    if len(sys.argv) == 2:
        s57 = S57(sys.argv[1])
    else:
        print 'usage s57.py infile'


