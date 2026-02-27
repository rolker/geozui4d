#!/usr/bin/python

import xml.dom.minidom
import xml.xpath
import os

import gz4d

import profile
import item
import itemCollection


class Plan:
    '''
    The Plan class contains a mission plan that can be described by
    the Profile specified in self.profile.
    '''
    def __init__(self,missionfn = None, profilefn = None):
        '''
        Create a new Plan. If a profile file name is supplied, attempt
        to load it. If a mission plan file name is supplied,	attempt
        to load it using the loaded profile, if one was loaded, or
        with a profile specified in the mission plan.
        @param missionfn: File name of the mission plan to load
        @type  missionfn: string
        @param profilefn: File name of the profile to load
        @type  profilefn: string
        '''

        self.gui = None
        
        self.gz = gz4d.Connection()
        try:
            self.gz.Connect()
        except gz4d.GzError:
            self.gz = None
            self.gzmp = None
            self.basePosition[0.0,0.0,0.0]
        else:
            self.gzmp = gz4d.ObjectConnection(15000,15001,self.gzMessage)
            self.basePosition = self.gz.GetPosition()
            cmd = ' '.join(('gz newobject MissionPlan mp',str(self.basePosition[0]),str(self.basePosition[1]),str(self.basePosition[2])))
            self.objid = self.gz.SendCommand(cmd)[1]
            self.gz.SendCommand(' '.join(('gz tellObject',self.objid,'connect',str(15001),str(15000))))
            

        self.filename = missionfn
        self.profile = None

        self.commands = itemCollection.ItemCollectionInstance(plan=self)
        self.sections = None


        if profilefn:
            self.LoadProfile(profilefn)

        if missionfn:
            self.Load()

    #def Idle(self):
        #if self.gzmp is not None:
            #m = self.gzmp.getData()
            #if m is not None:
                #self.gzMessage(m)

    def LoadProfile(self, pfn):
        p = profile.Profile()
        if p.Load(pfn):
            self.profile = p

            # profile loaded, now initialize plan
            self.commands = itemCollection.ItemCollectionInstance(profile=self.profile,plan=self)
            self.sections = itemCollection.ItemCollectionInstance(items = self.profile.sections,profile=self.profile,plan = self)

    def Load(self, fn=None):
        if fn:
            self.filename = fn
        doc = xml.dom.minidom.parse(self.filename)

        #if we don't have a profile yet, look for one
        if not self.profile:
            pfn = xml.xpath.Evaluate('string(//@mpprofile)',doc)
            if not pfn:
                pfn = 'auv.gzprofile'
            if not os.path.isfile(pfn) and self.gz:
                pfn = os.path.join(self.gz.appdir,'mpProfiles',pfn)
            self.LoadProfile(pfn)

        if self.profile and self.profile.isValid():
            #load the mission, guided by the profile

            #do we have the correct doc type?
            root = xml.xpath.Evaluate(''.join(('/',self.profile.top)),doc)
            if not root:
                doc.unlink()
                print 'Error reading mission'
                return
            root=root[0]

            self.sections.Load(root)

            gr = self.sections.gzTags['georef']

            if self.gz is not None:
                pos =''.join(('<position x="',gr.items.gzTags['locale'].value,'" y="',gr.items.gzTags['localn'].value,'" z="',str(0.0),'"/>'))
                cmd = ''.join(('<mp><setgeoref zone="',gr.items.gzTags['utmzone'].value,'">',pos,'</setgeoref></mp>'))
                self.gzmp.sendData(cmd)

                cmd = ' '.join(('<mp><setview>',pos,'</setview></mp>'))
                self.gzmp.sendData(cmd)

            #commands
            xp = []
            for k,c in self.profile.commands.iteritems():
                xp.append(c.attributes['path'])
            xp = '|'.join(xp)
            for c in xml.xpath.Evaluate(xp,root):
                self.AddCommand(cname=xml.xpath.Evaluate('name()',c),node=c)



        else:
            print "Problem finding a profile..."
            return


    def AddCommand(self,cname, node = None):
        c = item.ItemInstance(item = self.profile.commands[cname],profile = self.profile,plan=self)
        if node is not None:
            c.Load(node,True)
        else:
            if self.gzmp is not None:
                cmd = ''.join(('<mp><getpos id="',str(c.iid),'"/></mp>'))
                self.gzmp.sendData(cmd)
            try:
                c.items.gzTags['ex'].value = self.basePosition[0]
                c.items.gzTags['ey'].value = self.basePosition[1]
                c.items.gzTags['ez'].value = 0
            except KeyError:
                pass

        self.commands[c.iid] = c
        return c

    def gzMessage(self,msg):
        print 'gzMessage:',msg
        doc = xml.dom.minidom.parseString(msg)
        for mp in xml.xpath.Evaluate('/mp',doc):
            for c in mp.childNodes:
                cmd = c.nodeName
                #print 'command:',cmd
                if xml.xpath.Evaluate('boolean(@id)',c):
                    cid = xml.xpath.Evaluate('number(@id)',c)
                    #print cmd,cid
                    if cmd == 'select':
                        itemid = None
                        if xml.xpath.Evaluate('boolean(item/@id)',c):
                            itemid = xml.xpath.Evaluate('number(item/@id)',c)
                        if self.gui is not None:
                            self.gui.SelectCallback(cid, itemid)
                    else:
                        #try:
                        self.commands[cid].processCommand(c)
                        #except KeyError:
                        #    print 'unkown command id',cid,'in Plan::gzMessage'
                else:
                    if cmd == 'setgeoref':
                        gr = self.sections.gzTags['georef']
                        for p in xml.xpath.Evaluate('position',c):
                            self.basePosition[0] = xml.xpath.Evaluate('string(@x)',p)
                            self.basePosition[1] = xml.xpath.Evaluate('string(@y)',p)
                            gr.items.gzTags['locale'].setValue( self.basePosition[0])
                            gr.items.gzTags['localn'].setValue( self.basePosition[1])
                        for g in xml.xpath.Evaluate('geographic',c):
                            gr.items.gzTags['locallon'].setValue(xml.xpath.Evaluate('string(@longitude)',g))
                            gr.items.gzTags['locallat'].setValue(xml.xpath.Evaluate('string(@latitude)',g))

        doc.unlink()

    def updateDisplays(self):
        self.sections.updateDisplays()
        self.commands.updateDisplays()


    def __str__(self):
        tab = '   '

        ret = 'MissionPlan.Plan:\n'

        ret = ''.join((ret,tab,'filename: '))
        if self.filename:
            ret = ''.join((ret,self.filename,'\n'))
        else:
            ret = ''.join((ret,'(none)\n'))

        ret = ''.join((ret,tab,'Profile: '))
        if self.profile:
            ret = ''.join((ret,'yes\n'))
            ret = ''.join((ret,tab,tab,'Valid: '))
            if self.profile.isValid():
                ret = ''.join((ret,'yes\n'))
            else:
                ret = ''.join((ret,'no\n'))
        else:
            ret = ''.join((ret,'no\n'))

        ret = ''.join((ret,tab,'\nSections:\n',self.sections.PrettyString()))

        ret = ''.join((ret,tab,'\nCommands:\n',self.commands.PrettyString()))

        return ret



