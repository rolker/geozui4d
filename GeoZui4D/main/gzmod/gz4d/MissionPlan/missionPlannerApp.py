#!/usr/bin/python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import missionPlannerUI
import wx
import os

import xml.dom.minidom
import xml.xpath

import gz4d.MissionPlan


# Define notification event for select callback
EVT_SELECT_ID = wx.NewId()
EVT_VALUE_ID = wx.NewId()

class SelectEvent(wx.PyEvent):
    def __init__(self,cid, itemid):
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_SELECT_ID)
        self.cid = cid
        self.itemid = itemid

class ValueEvent(wx.PyEvent):
    def __init__(self,item):
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_VALUE_ID)
        self.item = item


class OptionsDialog(missionPlannerUI.OptionsDialogBase):
    '''
    Provides GUI to change general Mission Planner setting
    '''
    def __init__(self, *args, **kwds):
        missionPlannerUI.OptionsDialogBase.__init__(self, *args, **kwds)

    def OnDismissButton(self,evt):
        self.Show(False)

class MissionPlannerFrame(missionPlannerUI.MissionPlannerFrameBase):
    def __init__(self, *args, **kwds):
        missionPlannerUI.MissionPlannerFrameBase.__init__(self, *args, **kwds)
        self.options = OptionsDialog(self)
        self.detailsSizer = wx.BoxSizer(wx.VERTICAL)
        self.detailsWindow.SetSizerAndFit(self.detailsSizer)

        # Dictonaries returning gui control id for and item and vice-versa
        self.Controls = {}
        self.Items = {}

        self.Connect(-1, -1, eventType = EVT_SELECT_ID, func=self.OnSelect)
        self.Connect(-1, -1, eventType = EVT_VALUE_ID, func=self.OnValue)

        # if we were launched from GZ, connect so we can later get datadir value.
        self.gzc = gz4d.Connection()
        try:
            self.gzc.Connect()
        except gz4d.GzError:
            self.gzc = None


    def NewPlan(self,missionfn=None,profilefn=None):
        '''
        Create a new plan or load an exsisting one.
        '''

        self.mission = gz4d.MissionPlan.Plan(missionfn=missionfn,profilefn=profilefn)

        #self.timer = wx.PyTimer(self.mission.Idle)
        #self.timer.Start(50) #ms

        self.UpdatePaths()
        self.mission.gui = self

        self.CreateMissionGUI()

        #populate the list of commands in this mission
        for c in self.mission.commands.values():
            self.CreateCommandGUI(c)
        self.detailsSizer.Layout()

        self.UpdateEnabledStates()

        #populate the list of possible commands
        #cs = wx.GridSizer(rows=1,vgap=5,hgap=5)
        cs = wx.BoxSizer()
        for c in self.mission.profile.commands.keys():
            cb = wx.Button(self.addCommandPanel,label=c)
            try:
                cb.SetToolTipString(self.mission.profile.commands[c].attributes['help'])
            except KeyError:
                pass
            cs.Add(cb,flag=wx.EXPAND|wx.ADJUST_MINSIZE)
            self.Bind(wx.EVT_BUTTON, self.OnAddCommand,cb)
        self.addCommandPanel.SetSizer(cs)
        self.addCommandPanel.Layout()


    ##################################
    # GUI Creation Methods
    ##################################

    def CreateCommandGUI(self,cmd):
        cmdPanel = wx.ScrolledWindow(self.detailsWindow)
        cmdPanel.SetAutoLayout(True)

        self.commandList.Append(cmd.itemType.name,cmd)
        cmdgrid = wx.FlexGridSizer(rows=1, cols=3, vgap=3, hgap=3)
        cmdgrid.AddGrowableCol(1)
        if cmd.items is not None:
            sik = cmd.items.keys()[:]
            sik.sort()
            for ik in sik:
                i = cmd.items[ik]
                self.AddItemToGridSizer(i,cmdgrid,0,cmdPanel)

        cmdPanel.SetSizer(cmdgrid)
        self.Controls[cmd] = {'panel':cmdPanel}

        cmdgrid.Fit(cmdPanel)
        cmdgrid.SetSizeHints(cmdPanel)
        self.detailsSizer.Add(cmdPanel,flag = wx.ALL|wx.EXPAND,border = 2)
        self.detailsSizer.Show(cmdPanel,show=False,recursive=True)


    def CreateMissionGUI(self):

        sk = self.mission.sections.keys()[:]
        sk.sort()
        for k in sk:
            s = self.mission.sections[k]
            sizer = wx.FlexGridSizer(cols=3,vgap=3,hgap=3)
            sizer.AddGrowableCol(1)
            notebookPage = wx.ScrolledWindow(parent = self.sectionsNotebook)

            sik = s.items.keys()[:]
            sik.sort()
            for ik in sik:
                i = s.items[ik]
                self.AddItemToGridSizer(item = i, sizer = sizer, space = 0, parent = notebookPage)

            notebookPage.SetScrollRate(10, 10)
            notebookPage.SetAutoLayout(True)
            notebookPage.SetSizer(sizer)
            sizer.Fit(notebookPage)
            sizer.SetSizeHints(notebookPage)
            try:
                label = s.itemType.attributes['label']
            except KeyError:
                label = s.itemType.name
            self.sectionsNotebook.AddPage(notebookPage,label)

        self.missionPane.Layout()
        self.missionPane.FitInside()

    def AddItemToGridSizer(self,item,sizer,space,parent):
        try:
            l = item.itemType.attributes['label']
        except KeyError:
            l = item.itemType.name
        sizer.Add(wx.StaticText(parent,label=l),proportion=0,flag=wx.LEFT|wx.ALIGN_CENTER_VERTICAL,border = space*15)

        self.Controls[item] =  {}

        if item.items is not None:
            sizer.Add(wx.Size(1,40))
            self.AddItemEnabled(item,sizer,parent)

            for i in item.items.values():
                self.AddItemToGridSizer(i,sizer,space+1,parent)
        else:
            itemCtrl = None
            try:
                if item.itemType.attributes['type'] == 'bool':
                    itemCtrl = wx.CheckBox(parent)
            except KeyError:
                pass
            if not itemCtrl and len(item.itemType.possibleValues) > 0:
                s = wx.CB_DROPDOWN
                try:
                    if item.itemType.attributes['restricted']:
                        s = wx.CB_READONLY
                except KeyError:
                    pass
                itemCtrl = wx.ComboBox(parent,choices=item.itemType.possibleValues.keys(),style=s)
            if not itemCtrl:
                itemCtrl = wx.TextCtrl(parent)

            self.Controls[item]['id']=itemCtrl.GetId()
            self.Items[itemCtrl.GetId()] = item
            self.UpdateItemControl(item)

            sizer.Add(itemCtrl,1,flag = wx.EXPAND|wx.ADJUST_MINSIZE|wx.ALIGN_CENTER_VERTICAL)
            self.AddItemEnabled(item,sizer,parent)

    def AddItemEnabled(self,item,sizer,parent):
        try:
            r = item.itemType.attributes['required']
        except KeyError:
            r = False
        if r:
            sizer.Add(wx.Size(1,1))
        else:
            cb = wx.CheckBox(parent)
            cb.SetValue(item.enabled)
            sizer.Add(cb,0,flag=wx.ALIGN_CENTER_VERTICAL|wx.RIGHT|wx.LEFT,border=5)
            self.Bind(wx.EVT_CHECKBOX,self.OnItemEnabledCheck,cb)
            self.Controls[item]['cb'] = cb.GetId()
            self.Items[cb.GetId()] = item

    ##################################
    # GUI Update Methods
    ##################################

    def UpdatePaths(self):
        '''
        Update the labels showing the file paths of the mission and profile
        '''
        if self.mission:
            if self.mission.filename:
                self.MissionPathLabel.SetLabel("Mission: " + self.mission.filename)
            if self.mission.profile and self.mission.profile.isValid():
                self.ProfilePathLabel.SetLabel("Profile: " + self.mission.profile.filename)


    def UpdateEnabledStates(self):
        for s in self.mission.sections.values():
            for i in s.items.values():
                self.UpdateItemEnabled(i)

        for c in self.mission.commands.values():
            for i in c.items.values():
                self.UpdateItemEnabled(i)

    def UpdateItemEnabled(self,item,enabledParent = True):
        '''
        Determines if an item should be editable, therefore enabled.
        If an item itself is enabled, and also it's parent item, if
        it has one, then it should be enabled and editable.
        '''
        e = enabledParent and item.enabled

        try:
            self.FindWindowById(self.Controls[item]['id']).Enable(e)
        except KeyError:
            pass

        try:
            self.FindWindowById(self.Controls[item]['cb']).Enable(enabledParent)
        except KeyError:
            pass

        if item.items is not None:
            for i in item.items.values():
                self.UpdateItemEnabled(i,e)

    def UpdateItemControl(self,item):
        control = self.FindWindowById(self.Controls[item]['id'])

        if control.GetClassName() in ('wxSpinCtrl'):
            if item.value is not None:
                control.SetValueString(item.value)
            return

        if control.GetClassName() in ('wxCheckBox'):
            if item.value is not None:
                control.SetValue(item.value)
            return

        if item.value is not None:
            control.SetValue(str(item.value))
        else:
            control.SetValue('')
        self.UpdateEnabledStates()

    def ShowCommand(self, cmd, item):
        self.detailsSizer.ShowItems(False)
        self.detailsSizer.Show(self.Controls[cmd]['panel'],show=True,recursive=True)
        self.detailsWindow.Layout()
        self.detailsWindow.FitInside()
        cmd.setSelected(item)

    ##################################
    # Menu Event handlers
    ##################################

    def OnFileNew(self,evt):
        profileDialog = wx.FileDialog(self,style=wx.OPEN)
        if self.gzc is not None:
            profileDir = os.path.join(self.gzc.appdir,'mpProfiles')
            profileDialog.SetDirectory(profileDir)
        ret = profileDialog.ShowModal()
        if ret == wx.ID_OK:
            self.NewPlan(profilefn=profileDialog.GetPath())

    def OnFileOpen(self,evt):
        fdlg = wx.FileDialog(self,style=wx.OPEN)
        if self.gzc is not None:
            fdlg.SetDirectory(self.gzc.datadir)
        ret = fdlg.ShowModal()
        if ret == wx.ID_OK:
            self.NewPlan(missionfn=fdlg.GetPath())

    def OnFileExit(self,evt):
        self.Close()

    ##################################
    # GUI Event handlers
    ##################################

    def OnAddCommand(self,evt):
        c = self.mission.AddCommand(evt.GetEventObject().GetLabel())
        self.CreateCommandGUI(c)
        self.UpdateEnabledStates()

    def OnViewOptions(self,evt):
        self.options.Show()

    def OnShowCommand(self,evt):
        self.ShowCommand(self.commandList.GetClientData(self.commandList.GetSelection()),0)

    def OnItemEnabledCheck(self,evt):
        i = self.Items[evt.GetEventObject().GetId()]
        i.enabled = evt.GetEventObject().GetValue()
        self.UpdateEnabledStates()

    def OnSelect(self,evt):
        for i in range(self.commandList.GetCount()):
            if self.commandList.GetClientData(i).iid == evt.cid:
                self.commandList.SetSelection(i)
                self.ShowCommand(self.commandList.GetClientData(i),evt.itemid)
                break

    def OnValue(self,evt):
        self.UpdateItemControl(evt.item)

    ##################################
    # Mission Plan callbacks
    ##################################

    def SelectCallback(self,cid, itemid):
        wx.PostEvent(self,SelectEvent(cid,itemid))

    def ValueCallback(self,item):
        wx.PostEvent(self,ValueEvent(item))

class missionPlannerApp(wx.App):

    def OnInit(self):
        self.mpFrame = MissionPlannerFrame(None)
        self.mpFrame.Show()
        self.SetTopWindow(self.mpFrame)
        return True
