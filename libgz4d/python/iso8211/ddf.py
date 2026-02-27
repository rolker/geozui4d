#!/usr/bin/env python

import struct
import math

class DDF:
    def __init__(self, fname):
        self.infile = open(fname,'rb')
        self.infile.seek(0,2)
        self.eof = self.infile.tell()

        self.ddr = DDR(self.infile)
        self.reset()

    def reset(self):
        self.nextRecordPosition = self.ddr.leader.record_length

    def nextRecord(self):
        if self.nextRecordPosition < self.eof:
            ret = DR(self.infile,self.nextRecordPosition,self.ddr)
            self.nextRecordPosition += ret.leader.record_length
            return ret

    def xmlOpenTag(self):
        ret = '<DDF size="{0}">\n'.format(self.eof)
        ret += self.ddr.xmlString()
        return ret

    def xmlCloseTag(self):
        return '</DDF>'

    def xmlPrint(self):
        print self.xmlOpenTag()
        self.reset()
        r = self.nextRecord()
        while r is not None:
            print r.xmlString()
            r = self.nextRecord()
        print self.xmlCloseTag()

    def saveXML(self,fname):
        outfile = file(fname,'w')
        outfile.write(self.xmlOpenTag()+'\n')
        self.reset()
        r = self.nextRecord()
        while r is not None:
            d = r.xmlString()
            outfile.write(d+'\n')
            r = self.nextRecord()
        outfile.write(self.xmlCloseTag()+'\n')

class DR:
    FIELD_TERMINATOR = '\x1e'
    UNIT_TERMINATOR = '\x1f'

    def __init__(self, infile, start, ddr):
        self.infile = infile

        self.start = start
        infile.seek(self.start)
        leaderData = infile.read(24)
        self.leader = Leader(leaderData)

        self.tags = []
        self.directory = {}

        for i in range(self.leader.directorySize()):
            self.leader.ded.readEntry(infile,self.tags, self.directory)

        self.ddr = ddr

    def __str__(self):
        ret = 'start of record: '+str(self.start)+' leader: '+str(self.leader)
        for t in self.tags:
            ret += '\n\t'+t+': '+str(self.directory[t])
        return ret

    def size(self):
        return len(self.tags)


    def getField(self, i):
        return self.getFieldFromTag(self.tags[i])

    def getFieldFromTag(self, t):
        self.infile.seek(self.start+self.leader.base_address+self.directory[t][0])
        data = self.infile.read(self.directory[t][1])
        if data[-1] != DR.FIELD_TERMINATOR:
            print 'Field terminator not found where expected!'
        return data[:-1]

    def getData(self):
        return self.ddr.decode(self)

    def xmlOpenTag(self,tag='DR', extras=''):
        return '<{0} start="{1}"{2}>\n'.format(tag,self.start,extras)

    def xmlCloseTag(self,tag='DR'):
        return '</{0}>\n'.format(tag)

    def xmlContent(self):
        ret = self.leader.xmlString()+'\n'
        ret += '  <Directory>\n'
        for t in self.tags:
            ret += '    <Entry tag="{0}" pos="{1}" length="{2}"/>\n'.format(t,self.directory[t][0],self.directory[t][1])
        ret += '  </Directory>\n'
        ret += self.xmlData()
        return ret

    def xmlData(self):
        return self.xmlField(self.getData())

    def xmlField(self,data):
        ret = '<Field tag="{0}">\n'.format(data[0])
        for sfi in data[1]:
            ret += '<SubFields>\n'
            for sf in sfi:
                ret += '<SubField tag="{0}">{1}</SubField>\n'.format(sf[0],sf[1])
            ret += '</SubFields>\n'

        for c in data[2]:
            ret += self.xmlField(c)
        ret += '</Field>\n'
        return ret

    def xmlString(self):
        ret = self.xmlOpenTag()
        ret += self.xmlContent()
        ret += self.xmlCloseTag()
        return ret

class Leader:
    def __init__(self,data):
        self.record_length = int(data[0:5])
        self.leader_id = data[6]
        self.base_address = int(data[12:17])

        self.ded = DirectoryEntryDefinition(data)

        if self.leader_id == 'L':
            self.interchange_level = int(data[5])
            self.inline_code_extension = data[7]
            self.version = data[8]
            self.application_indicator = data[9]
            self.field_ctrl_length = int(data[10:12])
            self.extended_char_set = data[17:20]

    def __str__(self):
        ret = 'record length: '+str(self.record_length)+' leader id: '+self.leader_id+' base address: '+str(self.base_address)
        if self.leader_id == 'L':
            ret += ' version: '+self.version+' field ctrl len: '+str(self.field_ctrl_length)+' interchange lvl: '+str(self.interchange_level)
        return ret

    def directorySize(self):
        dir_len = self.base_address - 24
        return dir_len/self.ded.size()

    def xmlString(self):
        ret = '<Leader record_length="{0}" leader_id="{1}" base_address="{2}" '.format(self.record_length,self.leader_id,self.base_address)
        ret += self.ded.xmlString()
        if self.leader_id == 'L':
            ret += ' interchange_level="{0}" inline_code_extension="{1}" version="{2}"'.format(self.interchange_level,self.inline_code_extension, self.version)
            ret += ' application_indicator="{0}" field_ctrl_length="{1}" extended_char_set="{2}"'.format(self.application_indicator,self.field_ctrl_length,self.extended_char_set)
        ret += '/>'
        return ret


class DirectoryEntryDefinition:
    def __init__(self,leaderData):
        self.tag_size = int(leaderData[23])
        self.pos_size = int(leaderData[21])
        self.len_size = int(leaderData[20])

    def size(self):
        return self.tag_size+self.pos_size+self.len_size

    def readEntry(self,infile,tags,outdict):
        tag = infile.read(self.tag_size)
        length = int(infile.read(self.len_size))
        pos = int(infile.read(self.pos_size))
        tags.append(tag)
        outdict[tag]=(pos,length)

    def xmlString(self):
        return ' tag_size="{0}" pos_size="{1}" len_size="{2}"'.format(self.tag_size,self.pos_size,self.len_size)

class FieldDefinition:
    data_structure_codes = {'0':'elementry',
                           '1':'vector',
                           '2':'array',
                           '3':'concatenated'}

    data_type_codes = {'0':'string',
                      '1':'implicit point',
                      '5':'binary',
                      '6':'mixed'}

    def __init__(self,tag,field_ctrls,data):
        self.tag = tag
        self.children = []
        self.name = data[0]

        data_structure = field_ctrls[0]
        data_type = field_ctrls[1]
        aux_ctrls = field_ctrls[2:4]
        ft = field_ctrls[4]
        ut = field_ctrls[5]
        esc_seq = None
        if len(field_ctrls) > 6:
            esc_seq = field_ctrls[6:]

        #print 'stucture:',FieldDefinition.data_structure_codes[data_structure]
        #print 'type:',FieldDefinition.data_type_codes[data_type]

        self.static_labels = None
        self.array_labels = None

        labs = data[1]

        if len(labs) > 0:
            if '*' in labs:
                s,a = labs.split('*',1)
                self.array_labels = a.split('!')
            else:
                s = labs
            if len(s) > 0:
                self.static_labels = s.split('!')


        self.formats = []
        subs = data[2]
        if len(subs) > 2:
            if subs[0] == '(' and subs[-1] == ')':
                subs = subs[1:-1]
            sfds =  subs.split(',')
            for d in sfds:
                sfd = SubfieldDefinition(d)
                for i in range(sfd.count):
                    self.formats.append(sfd)
        else:
            self.formats = None

    def addChild(self,tag,field_ctrls,data):
        self.children.append(FieldDefinition(tag,field_ctrls,data))
        return self.children[-1]

    def decode(self, rec):
        if rec.directory.has_key(self.tag):
            return Field(self,rec)


    def printDebug(self,tabs=0):
        print '\t'*tabs+self.tag+': '+self.name
        if self.labels is not None:
            for i in range(len(self.labels)):
                print '\t'*(tabs+1),self.labels[i],':',self.formats[i]
        for c in self.children:
            c.printDebug(tabs+1)

    def xmlString(self):
        ret = '<FieldDefinition tag="{0}" name="{1}">\n'.format(self.tag,self.name)
        if self.labels is not None:
            for i in range(len(self.labels)):
                ret += self.formats[i].xmlString(self.labels[i])
        for c in self.children:
            ret += c.xmlString()
        ret += '</FieldDefinition>\n'
        return ret

class Field:
    def __init__(self,fd,rec):
        self.tag = fd.tag
        self.subFields = ([],[])
        self.children = []
        data = rec.getFieldFromTag(fd.tag)

        if fd.static_labels is not None:
            #print 'static labels',fd.static_labels
            for i in range(len(fd.static_labels)):
                sfd = fd.formats[i]
                value,data = sfd.decode(data)
                self.subFields[0].append((fd.static_labels[i],value))
        if fd.array_labels is not None:
            while len(data) > 0:
                #print 'array labels',fd.array_labels
                self.subFields[1].append([])
                for i in range(len(fd.array_labels)):
                    sfd = fd.formats[i]
                    value,data = sfd.decode(data)
                    self.subFields[1][-1].append((fd.array_labels[i],value))
        if fd.static_labels is None and fd.array_labels is None:
            value,data = fd.formats[0].decode(data)
            self.subFields[0].append((None,value))

        for c in fd.children:
            cret = c.decode(rec)
            if cret is not None:
                self.children.append(cret)

    def Get(self,label=None,row=None):
        if self.tag == label:
            return self

        if row is None:
            for sf in self.subFields[0]:
                if sf[0] == label:
                    return sf[1]
        else:
            if len(self.subFields[1]) > row:
                for sf in self.subFields[1][row]:
                    if sf[0] == label:
                        return sf[1]

        for c in self.children:
            ret = c.Get(label, row)
            if ret is not None:
                return ret

    def rowCount(self):
        return len(self.subFields[1])

    def __str__(self):
        ret = '{0}:\n'.format(self.tag)
        sf_strs=[]
        for sf in self.subFields[0]:
            sf_strs.append('{0}:{1}'.format(sf[0],sf[1]))
        ret += '  ({0})\n'.format(','.join(sf_strs))
        for sfai in self.subFields[1]:
            sf_strs=[]
            for sf in sfai:
                sf_strs.append('{0}:{1}'.format(sf[0],sf[1]))
            ret += '    ({0})\n'.format(','.join(sf_strs))
        for c in self.children:
            ret += '  '+'\n  '.join(str(c).splitlines())+'\n'
        return ret

class SubfieldDefinition:
    s_formats = {'1':{1:'B',2:'H',4:'I'},
                 '2':{1:'b',2:'h',4:'i'}}

    def __init__(self,d):
        self.count = 1
        if d[0].isdigit():
            self.count = int(d[0])
            d = d[1:]
        self.s_pattern = None
        self.a_type = None
        self.fixed_size = None

        if d[0] == 'b':
            s_type = d[1]
            self.fixed_size = int(d[2])
            self.s_pattern = '<'+SubfieldDefinition.s_formats[s_type][self.fixed_size]
        elif d[0] in ('A','I','R','B'):
            self.a_type = d[0]
            if len(d) > 1:
                size = d[2:-1]
                if len(size) > 0:
                    self.fixed_size = int(size)
                    if d[0] == 'B':
                        self.fixed_size = int(math.ceil(self.fixed_size/8.0))

        else:
            raise Exception('unknown format')


    def decode(self,data):
        if self.s_pattern is not None:
            #print 'fixed size:',self.fixed_size,'data len',len(data)
            return struct.unpack(self.s_pattern,data[:self.fixed_size])[0],data[self.fixed_size:]
        elif self.fixed_size is not None:
            if self.a_type == 'B':
                ret = 0
                for i in range(self.fixed_size):
                    ret = ret*256+ord(data[0])
                    data = data[1:]
                return ret,data
            return data[:self.fixed_size],data[self.fixed_size:]
        else:
            return data.split(DR.UNIT_TERMINATOR,1)

    def __repr__(self):
        ret = 'struct pattern: '+str(self.s_pattern)+' ascii type: '+str(self.a_type)+' fixed size: '+str(self.fixed_size)
        return ret

    def xmlString(self,label):
        return '<SubfieldDefinition label="{0}" struct_pattern="{1}" ascii_type="{2}" fixed_size="{3}"/>\n'.format(label, '&lt;'.join(str(self.s_pattern).split('<')),self.a_type,self.fixed_size)

class DDR(DR):
    def __init__(self, infile):
        DR.__init__(self,infile,0,None)

        tag_size = self.leader.ded.tag_size

        file_ctrl_field_tag = '0'*tag_size
        root_tag = file_ctrl_field_tag[:-1]+'1'

        defs = {}

        for i in range(len(self.tags)):
            tag = self.tags[i]
            data = self.getField(i)
            field_ctrls = data[:self.leader.field_ctrl_length]
            data = data[self.leader.field_ctrl_length:].split(DR.UNIT_TERMINATOR)

            if tag == file_ctrl_field_tag:
                pair_size = tag_size*2
                pair_count = len(data[1])/pair_size
                parents = {}
                for p in range(pair_count):
                    pc = data[1][p*pair_size:(p+1)*pair_size]
                    parent = pc[:tag_size]
                    child = pc[tag_size:]
                    parents[child] = parent
            elif tag == root_tag:
                self.root = FieldDefinition(tag,field_ctrls,data)
                defs[root_tag] = self.root
            else:
                defs[tag] = defs[parents[tag]].addChild(tag,field_ctrls,data)

    def decode(self, rec):
        return self.root.decode(rec)

    def xmlOpenTag(self):
        return DR.xmlOpenTag(self,'DDR')

    def xmlCloseTag(self):
        return DR.xmlCloseTag(self,'DDR')

    def xmlData(self):
        ret = ''
        for t in self.tags:
            d = '&amp;'.join(self.getFieldFromTag(t).split('&'))
            ret += '<Field tag="{0}">{1}</Field>\n'.format(t,d.split(DR.UNIT_TERMINATOR))
        return ret + self.root.xmlString()

if __name__ == '__main__':
    import sys
    if len(sys.argv) >= 2:
        ddf = DDF(sys.argv[1])
        if len(sys.argv) == 2:
            ddf.xmlPrint()
            sys.exit(0)
        if len(sys.argv) == 3:
            ddf.saveXML(sys.argv[2])
            sys.exit(0)

    print 'usage ddf.py infile'
    sys.exit(1)
