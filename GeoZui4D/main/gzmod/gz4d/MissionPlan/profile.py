#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import xml.dom.minidom
import xml.xpath
import gz4d
import itemCollection

###################################################
#
# MissionPlan.Profile
#
###################################################

class Profile:
    def __init__(self,filename = None):
        '''
        Create a profile. If a filename is supplied, {load} will be called
        to attempt to load it.

        @param filename: Filename of a profile to load. (Optional)
        @type  filename: string
        '''
        self.filename = filename

        self.Clear()

        self.str_attribs = ('name','help','type','default','gz','path')
        self.bool_attribs = ('attribute','element','required','restricted')

        if filename:
            self.Load()

    def Clear(self):
        '''
        Initialize the profile to None for all profile elements.
        '''
        self.top = None
        self.positiveDepths = None
        self.types = None
        self.sections = None
        self.commands = None

        self.valid = False


    def Load(self,fn = None):
        '''
        If self.filename contains a filename, load the profile 
        at the referred location. If a filename is supplied
        as an argument, it will first be assigned to self.filename.

        @param fn: Filename to assign to self.filename, which will 
                then be the one loaded. (Optional)
        @type  fn: string

        @return: True if profile was succesfully loaded, False otherwise.
        '''

        self.Clear()

        if fn:
            self.filename = fn

        if self.filename:
            doc = xml.dom.minidom.parse(self.filename)

            mp = xml.xpath.Evaluate('/mission_profile',doc)

            if mp:
                mp = mp[0]
                self.positiveDepths = xml.xpath.Evaluate('boolean(@positiveDepths)',mp)
                self.top = xml.xpath.Evaluate('string(@top)',mp)
                self.trueLabel = xml.xpath.Evaluate('string(@true)',mp)
                self.falseLabel = xml.xpath.Evaluate('string(@false)',mp)

                #user defined types
                self.types = itemCollection.ItemCollection()
                self.types.Read('type',mp)

                # sections
                self.sections = itemCollection.ItemCollection()
                self.sections.Read('section',mp)

                #commands
                self.commands = itemCollection.ItemCollection()
                self.commands.Read('command',mp)

                self.valid = True
                doc.unlink()
                # assuming that getting here means a valid profile was loaded
                # proper profile validation should be done (something like: return self.validate())
                return True
            else:
                print "Document doesn't seem to be a mission profile"
        return False


    def isValid(self):
        return self.valid


    def __str__(self):
        tab = '   '

        ret = 'MissionPlan.Profile:\n'

        ret = ''.join((ret,tab,'filename: '))
        if self.filename:
            ret = ''.join((ret,self.filename,'\n'))
        else:
            ret = ''.join((ret,'(none)\n'))

        ret = ''.join((ret,tab,'Valid: ',str(self.valid),'\n'))

        ret = ''.join((ret,tab,'Positive Depths: ',str(self.positiveDepths),'\n'))

        ret = ''.join((ret,tab,'Top: '))
        if self.top:
            ret = ''.join((ret,self.top,'\n\n'))
        else:
            ret = ''.join((ret,'(none)\n\n'))

        ret = ''.join((ret,'Types:\n',self.types.PrettyString(),'\n'))
        ret = ''.join((ret,'Sections:\n',self.sections.PrettyString(),'\n'))
        ret = ''.join((ret,'Commands:\n',self.commands.PrettyString(),'\n'))

        return ret


