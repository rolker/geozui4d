#!/usr/bin/env python

import os

class Headers:
    def __init__(self,inc_dir = 'include/'):
        self.hfiles = {}

        for root, dirs, files in os.walk(inc_dir):
            for f in files:
                if f.endswith('.h'):
                    deps = {'headers':[],'deps':[]}
                    hf = open(os.path.join(root,f))
                    for l in hf.readlines():
                        if l.startswith('//gz4d_cmake'):
                            parts = l.split()
                            if len(parts) > 2 and parts[1] == 'depends':
                                deps['deps'] += parts[2:]
                            else:
                                print 'skipping',l.strip()
                                print '\tin',os.path.join(root,f)
                    ah,rh = self.getHeaders(os.path.join(root,f))
                    for h in ah:
                        if not h in deps['headers']:
                            deps['headers'].append(h)
                    for h in rh:
                        a = os.path.relpath(os.path.normpath(os.path.join(root,h)),inc_dir)
                        if not a in deps['headers']:
                            deps['headers'].append(a)
                        
                    self.hfiles[os.path.join(os.path.relpath(root,inc_dir),f)]=deps
                    
        
            if '.svn' in dirs:
                dirs.remove('.svn')

    def getHeaders(self,f):
        ah = []
        rh = []
        for l in open(f).readlines():
            if l.strip().startswith('#'):
                pl = l.strip()[1:].strip()
                if pl.startswith('include'):
                    il = pl[len('include'):].strip()
                    if il[0] == '<':
                        inc = il[1:].split('>',1)[0]
                        if inc.startswith('gz4d/'):
                            if not inc in ah:
                                ah.append(inc)
                    if il[0] == '"':
                        linc = il[1:].split('"',1)[0]
                        if not linc in rh:
                            rh.append(linc)
        return ah,rh
                
    def findDeps(self,h,visited=None):
        if visited is None:
            v = []
        else:
            v = visited[:]
        if not h in v:
            #print h
            if self.hfiles.has_key(h):
                ret = self.hfiles[h]['deps'][:]
                v.append(h)
                for sh in self.hfiles[h]['headers']:
                    sub_deps = self.findDeps(sh,v)
                    if sub_deps is not None:
                        for sd in sub_deps:
                            if not sd in ret:
                                ret.append(sd)
                #print '\t',ret
                return ret

    def findDepsInFiles(self,files,visited = None):
        if visited is None:
            v = []
        else:
            v = visited[:]
        ret = []
        for f in files:
            if not f in v:
                ah,rh = self.getHeaders(f)
                v.append(f)
                
                for gh in ah:
                    deps = self.findDeps(gh,v)
                    if deps is not None:
                        for d in deps:
                            if not d in ret:
                                ret.append(d)
                rhs = []
                for h in rh:
                    rhs.append(os.path.normpath(os.path.join(os.path.split(f)[0],h)))
                
                deps = self.findDepsInFiles(rhs,v)
                if deps is not None:
                    for d in deps:
                        if not d in ret:
                            ret.append(d)

        return ret
