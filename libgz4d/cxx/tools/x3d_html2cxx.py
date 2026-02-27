#!/usr/bin/env python
# coding: utf-8

#import xml.etree.ElementTree
import sys
import os
import HTMLParser
import math

typemap = '''
Node         gz4d::x3d::sai::Node
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

class ParseError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value

class CodeGenerator:

    def initHeader(self,hdir,name):
        fname = name+'.h'
        out = open(os.path.join(hdir,fname),'w')

        out.write('#ifndef GZ4D_X3D_'+name.upper()+'_H\n')
        out.write('#define GZ4D_X3D_'+name.upper()+'_H\n')
        out.write('\n')
        if name != 'Fields':
            out.write('#include<gz4d/x3d/ng/Fields.h>\n')
        return out

    def openNamespace(self,out):
        out.write('namespace gz4d\n')
        out.write('{\n')
        out.write('    namespace x3d\n')
        out.write('    {\n')

    def closeNamespace(self,out):
        out.write('    }\n')
        out.write('}\n')
        out.write('\n')
        out.write('#endif\n')
        out.write('\n')


    def initImplementation(self,cdir,name):
        fname = name+'.cpp'
        out = open(os.path.join(cdir,fname),'w')

        out.write('#define GZ4D_INTERNAL\n')
        out.write('#include <gz4d/x3d/ng/'+name+'.h>\n')
        out.write('\n')

        return out

class Field:
    def __init__(self,line):
        if not '[' in line:
            raise ParseError(line)

        self.type,rest = line.split('[',1)
        self.type = self.type.strip()

        if self.type == 'SFBoolean':
            self.type = 'SFBool'

        if not self.type.startswith('X3D') and not self.type.startswith('SF') and not self.type.startswith('MF'):
            raise ParseError(line)

        if '<' in self.type or '/' in self.type or self.type == 'fieldType':
            raise ParseError(line)

        access,rest = rest.split(']',1)

        self.input = 'in' in access
        self.output = 'out' in access
        self.interface = not((self.input and self.output) or (not self.input and not self.output))

        parts = rest.split(None,1)
        self.name = parts[0]

        if len(parts) == 2:
            rest = parts[1]
        else:
            rest = None

        self.from_base = None
        self.default = None
        self.restriction = None

        if rest is not None and not self.interface:

            if len(rest):
                if rest[0] == '[':
                    d,r = rest[1:].split(']',1)
                    if len(d):
                        self.default = d
                    if len(r.strip()):
                        self.restriction = r.strip()
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
                        self.default = rest
                    else:
                        if d1 is not None and d2 is not None:
                            if len(d1) < len(d2):
                                if len(d1.strip()):
                                    self.default = d1.strip()
                                self.restriction = r1
                            else:
                                if len(d2.strip()):
                                    self.default = d2.strip()
                                self.restriction = r2
                        else:
                            if d2 is None:
                                if len(d1.strip()):
                                    self.default = d1.strip()
                                self.restriction = r1
                            else:
                                if len(d2.strip()):
                                    self.default = d2.strip()
                                self.restriction = r2

                if self.default in ('NULL','[NULL]'):
                    self.default = None

                if self.default is not None:
                    if self.type == 'SFFloat':
                        if '/' in self.default:
                            a,b = self.default.split('/',1)
                            a = float(a)
                            b = float(b)
                            self.default = str(a/b)
                    if self.type == 'SFBool':
                        self.default = self.default.lower()

                    self.default = '\\"'.join(self.default.split('"'))
                    self.default = '"'+self.default+'"'

    def __str__(self):
        return  '\t'.join((self.type,self.name,'('+str(self.rest)+')'))

class Element (CodeGenerator):
    def __init__(self, lines):
        self.name = None
        self.bases = []
        self.fields = []
        self.derived = []
        
        #print lines


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
                    if ':' in l1:
                        name,bases = l1.split(':',1)
                        bases = bases.split('{')[0].strip().split(',')
                        for b in bases:
                            bs = b.strip()
                            if bs == 'X3DURLObject':
                                bs = 'X3DUrlObject'
                            self.bases.append(bs)
                    else:
                        name = l1.split('{')[0]

                    self.name = name.strip()
                    #print name,self.bases
            else:
                if '}' in l:
                    break
                else:
                    try:
                        self.fields.append(Field(l.strip()))
                    except ParseError, e:
                        print self.name,'- failed to parse field:',e

        if l1 is None:
            raise ParseError(''.join(lines))

    def __str__(self):
        ret = self.name
        if self.bases is not None:
            ret += ': ' + ', '.join(self.bases)
        for f in self.fields:
            ret += '\n\t'+str(f)
        return ret

    def getAllBases(self,x3d):
        ret = self.bases[:]
        for b in self.bases:
            for bb in x3d.elements[b].getAllBases(x3d):
                if not bb in ret:
                    ret.append(bb)
        return ret

    def writeHeader(self,hdir,x3d):
        out = self.initHeader(hdir,self.name)

        abstract = self.name.startswith('X3D')
        out.write('#include<gz4d/x3d/NodeInterface.h>\n')
        if not abstract:
            out.write('#include<gz4d/x3d/NodeData.h>\n')
        out.write('\n')

        self.openNamespace(out)

        if not abstract:
            # real node

            out.write('        class GZ4DEXPORT '+self.name+'Data: public NodeData\n')
            out.write('        {\n')
            out.write('            static sai::Field::DefinitionMap definitions;\n')
            out.write('            public:\n')
            out.write('                '+self.name+'Data();\n')
            out.write('                virtual std::string getTypeName() const;\n')
            out.write('                virtual void addNode(sai::Node n);\n')
            out.write('                virtual void set_field(std::string const &f, std::string const &v);\n')
            out.write('\n')
            for f in self.fields:
                if f.input:
                    out.write('                void set_'+f.name+'('+f.type+' const &);\n')
                if (f.input and f.output) or (not f.input and not f.output):
                    out.write('                '+f.type+' '+f.name+'();\n')

            out.write('\n')
            out.write('        };\n')
            out.write('\n')


        out.write('        class GZ4DEXPORT '+self.name+': public NodeInterface\n')
        out.write('        {\n')
        out.write('            public:\n')
        out.write('                '+self.name+'(boost::shared_ptr<NodeData> node);\n')
        out.write('                '+self.name+'(sai::Node node);\n')
        if not abstract:
            out.write('                boost::shared_ptr<'+self.name+'Data> operator->();\n')
        out.write('        };\n')
        out.write('\n')


        self.closeNamespace(out)


    def writeImplementation(self,cdir,x3d):
        out = self.initImplementation(cdir,self.name)

        abstract = self.name.startswith('X3D')
        # abstract node
        out.write('#include<gz4d/x3d/NodeData.h>\n')
        out.write('\n')

        out.write('gz4d::x3d::'+self.name+'::'+self.name+'(boost::shared_ptr<NodeData> node)\n')
        out.write('    :NodeInterface(node)\n')
        out.write('{\n')
        out.write('}\n')

        out.write('\n')

        out.write('gz4d::x3d::'+self.name+'::'+self.name+'(sai::Node n)\n')
        out.write('{\n')
        if abstract:
            out.write('    std::string name = n->getTypeName();\n')
            for d in self.derived:
                out.write('    if (name == "'+d+'")\n')
                out.write('    {\n')
                out.write('        node = n;\n')
                out.write('        return;\n')
                out.write('    }\n')
        else:
            out.write('    if (n->getTypeName() == "'+self.name+'")\n')
            out.write('        node = n;\n')
        out.write('}\n')

        out.write('\n')



        if not abstract:
            out.write('boost::shared_ptr<gz4d::x3d::'+self.name+'Data> gz4d::x3d::'+self.name+'::operator->()\n')
            out.write('{\n')
            out.write('    return boost::dynamic_pointer_cast<'+self.name+'Data>(node);\n')
            out.write('}\n')
            out.write('\n')

            out.write('gz4d::x3d::sai::Field::DefinitionMap gz4d::x3d::'+self.name+'Data::definitions;\n')
            out.write('\n')
            out.write('gz4d::x3d::'+self.name+'Data::'+self.name+'Data()\n')
            out.write('{\n')
            out.write('    if (definitions.empty())\n')
            out.write('    {\n')
            for f in self.fields:
                if f.input:
                    access = 'true,'
                else:
                    access = 'false,'
                if f.output:
                    access += 'true'
                else:
                    access += 'false'
                out.write('        definitions["'+f.name+'"]=sai::Field::Definition('+access+',"'+f.type+'","'+f.name+'");\n')
            out.write('    }\n')
            out.write('\n')
            for f in self.fields:
                if not f.interface:
                    out.write('        fields["'+f.name+'"]=boost::shared_ptr<'+f.type+' >(new '+f.type+');\n')
            out.write('}\n')
            out.write('\n')

            out.write('std::string gz4d::x3d::'+self.name+'Data::getTypeName() const\n')
            out.write('{\n')
            out.write('    return "'+self.name+'";\n')
            out.write('}\n')
            out.write('\n')

            out.write('void gz4d::x3d::'+self.name+'Data::'+'addNode(sai::Node n)\n')
            out.write('{\n')
            for f in self.fields:
                if f.type in ('SFNode','MFNode') and not f.interface and f.restriction is not None:
                    node_category = f.restriction.split('[',1)[1].split(']',1)[0]
                    cats = [node_category]
                    if('|' in node_category):
                        cats = node_category.split('|')
                    if(',' in node_category):
                        cats = node_category.split(',')

                    for cat in cats:
                        try:
                            for d in x3d.elements[cat.strip()].derived:
                                out.write('    if (n->getTypeName() == "'+d+'")\n')
                                out.write('    {\n')
                                if f.type == "SFNode":
                                    out.write('        (*boost::dynamic_pointer_cast<SFNode>(fields["'+f.name+'"]))=n;\n')
                                else:
                                    out.write('        boost::dynamic_pointer_cast<MFNode>(fields["'+f.name+'"])->append(n);\n')
                                out.write('        return;\n')
                                out.write('    }\n')
                        except KeyError:
                                pass
            for f in self.fields:
                if f.type in ('SFNode','MFNode') and not f.interface and f.restriction is None:
                    if f.type == "SFNode":
                        out.write('    (*boost::dynamic_pointer_cast<SFNode>(fields["'+f.name+'"]))=n;\n')
                    else:
                        out.write('    boost::dynamic_pointer_cast<MFNode>(fields["'+f.name+'"])->append(n);\n')



            out.write('}\n')
            out.write('\n')


            out.write('void gz4d::x3d::'+self.name+'Data::set_field(std::string const &f, std::string const &v)\n')
            out.write('{\n')
            out.write('    (*fields[f])=v;\n')
            out.write('}\n')
            out.write('\n')

            for f in self.fields:
                if f.input:
                    out.write('void gz4d::x3d::'+self.name+'Data::set_'+f.name+'('+f.type+' const &v)\n')
                    out.write('{\n')
                    out.write('    (*fields["'+f.name+'"])=v;\n')
                    out.write('}\n')

                if (f.input and f.output) or (not f.input and not f.output):
                    out.write('gz4d::x3d::'+f.type+' gz4d::x3d::'+self.name+'Data::'+f.name+'()\n')
                    out.write('{\n')
                    out.write('    return *boost::dynamic_pointer_cast<'+f.type+'>(fields["'+f.name+'"]);\n')
                    out.write('}\n')



class X3DParser(HTMLParser.HTMLParser):
    def __init__(self,x3d):
        HTMLParser.HTMLParser.__init__(self)
        self.node = None
        self.x3d = x3d

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
                e = Element(self.node)
                self.x3d.elements[e.name] = e
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
                n = str(math.pi)

            if n is not None:
                self.node += n
            else:
                self.node += 'unknown#:' + name

    def handle_entityref(self,name):
        if self.node is not None:
            n = None
            if name == "pi":
                n = str(math.pi)
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


class X3D (CodeGenerator):
    def __init__(self,fname):
        self.elements = {}
        self.components = []

        for fn in open(fname):
            component = os.path.splitext(os.path.basename(fn))[0]
            print '***',component,'***'

            self.components.append(component)
            p = X3DParser(self)
            try:
                p.feed(open(fn.strip()).read())
            except HTMLParser.HTMLParseError, e:
                print e

        for e in self.elements.itervalues():
            if not e.name.startswith('X3D'):
                for b in e.getAllBases(self):
                    if not e.name in self.elements[b].derived:
                        self.elements[b].derived.append(e.name)


    def writeHeader(self,hdir):
        for e in self.elements.itervalues():
            e.writeHeader(hdir,self)

        out = self.initHeader(hdir,'SAIExecutionContextBase')
        out.write('#include <gz4d/x3d/sai/Node.h>\n')
        self.openNamespace(out)

        out.write('        class GZ4DEXPORT SAIExecutionContextBase\n')
        out.write('        {\n')
        out.write('            enum NodeTypes {\n')
        for e in self.elements.itervalues():
            if not e.name.startswith('X3D'):
                out.write('                '+e.name+'_NodeType,\n')
        out.write('            };\n')
        out.write('            static std::map<std::string,NodeTypes> str2nodeType;\n')
        out.write('            protected:\n')
        out.write('                SAIExecutionContextBase();\n')
        out.write('            public:\n')
        out.write('                sai::Node createNode(std::string const &node_type);\n')
        out.write('        };\n')
        self.closeNamespace(out)


        out = self.initHeader(hdir,'Fields')
        out.write('#include<gz4d/x3d/FieldValue.h>\n')
        out.write('#include<gz4d/x3d/sai/Node.h>\n')
        out.write('#include<gz4d/Vector.h>\n')
        out.write('#include<gz4d/Time.h>\n')
        out.write('#include<gz4d/Matrix.h>\n')
        out.write('#include<gz4d/x3d/Image.h>\n')
        out.write('\n')

        self.openNamespace(out)

        for t in typemap.splitlines():
            if len(t):
                xtype,ctype = t.split(None,1)
                out.write('        class GZ4DEXPORT SF'+xtype+': public FieldValue\n')
                out.write('        {\n')
                out.write('            '+ctype+' value;\n')
                out.write('            public:\n')
                out.write('                void operator=(std::string const &);\n')
                if ctype != 'std::string':
                    out.write('                void operator=('+ctype+' const &);\n')
                out.write('                operator '+ctype+' &();\n')
                out.write('                operator '+ctype+' const &() const;\n')
                out.write('        };\n')
                out.write('\n')
    
                out.write('        class GZ4DEXPORT MF'+xtype+': public FieldValue\n')
                out.write('        {\n')
                out.write('            std::vector<'+ctype+' > values;\n')
                out.write('            public:\n')
                out.write('                typedef std::vector<'+ctype+' >::iterator iterator;\n')
                out.write('                void operator=(std::string const &);\n')
                out.write('                void append('+ctype+' const &);\n')
                out.write('                std::vector<'+ctype+' > const * operator->() const;\n')
                out.write('        };\n')
                out.write('\n')

        self.closeNamespace(out)



    def writeImplementation(self,cdir):
        for e in self.elements.itervalues():
            e.writeImplementation(cdir,self)

        out = self.initImplementation(cdir,'SAIExecutionContextBase')

        for e in self.elements.itervalues():
            if not e.name.startswith('X3D'):
                out.write('#include <gz4d/x3d/ng/'+e.name+'.h>\n')
        out.write('\n')

        out.write('std::map<std::string,gz4d::x3d::SAIExecutionContextBase::NodeTypes> gz4d::x3d::SAIExecutionContextBase::str2nodeType;\n')
        out.write('\n')
        
        out.write('gz4d::x3d::SAIExecutionContextBase::SAIExecutionContextBase()\n')
        out.write('{\n')
        out.write('    if(str2nodeType.empty())\n')
        out.write('    {\n')
        for e in self.elements.itervalues():
            if not e.name.startswith('X3D'):
                out.write('        str2nodeType["'+e.name+'"]='+e.name+'_NodeType;\n')
        out.write('    }\n')
        out.write('}\n')
        out.write('\n')

        out.write('gz4d::x3d::sai::Node gz4d::x3d::SAIExecutionContextBase::createNode(std::string const &node_type)\n')
        out.write('{\n')
        out.write('    switch(str2nodeType[node_type])\n')
        out.write('    {\n')
        for e in self.elements.itervalues():
            if not e.name.startswith('X3D'):
                out.write('        case '+e.name+'_NodeType:\n')
                out.write('        {\n')
                out.write('            boost::shared_ptr<'+e.name+'Data> n(new '+e.name+'Data);\n')
                out.write('            return sai::Node(n);\n')
                out.write('        }\n')
        out.write('        default:\n')
        out.write('            return sai::Node();\n')
        out.write('    }\n')
        out.write('    return sai::Node();\n')
        out.write('}\n')
        out.write('\n')


        out = self.initImplementation(cdir,'Fields')
        out.write('#include <boost/algorithm/string.hpp>\n')
        out.write('#include <gz4d/vector/ios.h>\n')
        for t in typemap.splitlines():
            if len(t):
                xtype,ctype = t.split(None,1)
                out.write('void gz4d::x3d::SF'+xtype+'::operator=(std::string const &s)\n')
                out.write('{\n')
                if xtype != 'Node':
                    out.write('    std::istringstream is(s);\n')
                    out.write('    is >> value;\n')
                out.write('};\n')
                out.write('\n')

                if ctype != 'std::string':
                    out.write('void gz4d::x3d::SF'+xtype+'::operator=('+ctype+' const &v)\n')
                    out.write('{\n')
                    out.write('    value = v;\n')
                    out.write('};\n')
                    out.write('\n')

                out.write('gz4d::x3d::SF'+xtype+'::operator '+ctype+' &()\n')
                out.write('{\n')
                out.write('    return value;\n')
                out.write('};\n')
                out.write('\n')

                out.write('gz4d::x3d::SF'+xtype+'::operator '+ctype+' const &() const\n')
                out.write('{\n')
                out.write('    return value;\n')
                out.write('};\n')
                out.write('\n')

                out.write('void gz4d::x3d::MF'+xtype+'::operator=(std::string const &s)\n')
                out.write('{\n')
                if xtype != 'Node':
                    out.write('    std::vector<std::string> parts;\n')
                    out.write('    boost::split(parts,s,boost::algorithm::is_any_of(","));\n')
                    out.write('    if (parts.size() >= 2)\n')
                    out.write('    {\n')
                    out.write('        for(std::vector<std::string>::iterator i = parts.begin(); i != parts.end(); ++i)\n')
                    out.write('        {\n')
                    out.write('            std::istringstream datastrm(*i);\n')
                    out.write('            if(datastrm)\n')
                    out.write('            {\n')
                    out.write('                '+ctype+' v;\n')
                    out.write('                datastrm >> v;\n')
                    out.write('                values.push_back(v);\n')
                    out.write('            }\n')
                    out.write('        }\n')
                    out.write('    }\n')
                    out.write('    else\n')
                    out.write('    {\n')
                    out.write('        std::istringstream datastrm(s);\n')
                    out.write('        while(datastrm.peek()>=0) //there were no commas, so assume the user put in the right amount in the right places!\n')
                    out.write('        {\n')
                    out.write('            '+ctype+' v;\n')
                    out.write('            datastrm >> v;\n')
                    out.write('            values.push_back(v);\n')
                    out.write('        }\n')
                    out.write('    }\n')
                out.write('};\n')
                out.write('\n')

                out.write('void gz4d::x3d::MF'+xtype+'::append('+ctype+' const &v)\n')
                out.write('{\n')
                out.write('    values.push_back(v);\n')
                out.write('};\n')
                out.write('\n')

                out.write('std::vector<'+ctype+' > const * gz4d::x3d::MF'+xtype+'::operator->() const\n')
                out.write('{\n')
                out.write('    return &values;\n')
                out.write('};\n')
                out.write('\n')

def usage():
    print 'usage: x3d_html2cxx.py [key=value ...]'
    print '     keys: hdir,cdir'
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

    x3d.writeHeader(params['hdir'])
    x3d.writeImplementation(params['cdir'])
