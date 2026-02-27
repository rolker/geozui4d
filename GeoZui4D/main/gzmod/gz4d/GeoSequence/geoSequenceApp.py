#!/usr/bin/python

from geoSequenceUI import *
import gz4d
import xml.dom.minidom
import xml.xpath
import datetime
import calendar

class GeoSequenceDialog(GeoSequenceDialogBase):
	def __init__(self, *args, **kwds):
		GeoSequenceDialogBase.__init__(self, *args, **kwds)
		
		self.gz = gz4d.Connection()
		self.gz.Connect()
		
	def OnOpenButton(self, evt):
		fdlg = wx.FileDialog(self,style=wx.OPEN,wildcard='*.xml')
		fdlg.SetDirectory(self.gz.datadir)
		ret = fdlg.ShowModal()
		if ret == wx.ID_OK:
			fname = fdlg.GetPath()
			self.filenameLabel.SetLabel(fname)
			self.Open(fname)

	def Open(self,fname):
		doc = xml.dom.minidom.parse(fname)
		self.date = xml.xpath.Evaluate('string(/BehavioralSequence/@date)',doc).strip()
		self.dateTextCtrl.SetValue(self.date)
		self.detailTextControl.SetValue(xml.xpath.Evaluate('string(BehavioralSequence)',doc).strip())
		wc = xml.xpath.Evaluate('count(/BehavioralSequence/Whale)',doc)
		self.whales = {}
		self.whaleNames = []
		for i in range(wc):
			whale=xml.xpath.Evaluate(''.join(('/BehavioralSequence/Whale[',str(i+1),']')),doc)[0]
			wname=xml.xpath.Evaluate('string(@name)',whale)
			self.whaleNames.append(wname)
			self.whales[wname] = {'behaviors':[]}
			bc = xml.xpath.Evaluate('count(Behavior)',whale)
			for b in range(bc):
				behavior = xml.xpath.Evaluate(''.join(('Behavior[',str(b+1),']')),whale)[0]
				self.whales[wname]['behaviors'].append([xml.xpath.Evaluate('string(@time)',behavior),xml.xpath.Evaluate('string(@code)',behavior),xml.xpath.Evaluate('string(@comment)',behavior)])
		self.detailsNotebook.DeleteAllPages()
		for n in self.whaleNames:
			page = wx.NotebookPage(self.detailsNotebook)
			self.detailsNotebook.AddPage(page,n)
			psizer = wx.BoxSizer(wx.HORIZONTAL)
			page.SetSizer(psizer)
			blist = wx.ListBox(page,style=wx.LB_SINGLE|wx.LB_SORT)
			for b in self.whales[n]['behaviors']:
				t = b[0].strip().split(':')
				d = self.date.split('/')
				try:
					dt = datetime.datetime(int(d[2]),int(d[0]),int(d[1]),int(t[0]),int(t[1]),int(t[2]))
					gztime = str(calendar.timegm(dt.utctimetuple())+(3600*4)) # +4Hrs for EDT->UTC
					self.whales[n]['gztime'] = gztime
				except ValueError:
					gztime = ''
				blist.Append('\t'.join(b),gztime)
			psizer.Add(blist)
			self.Bind(wx.EVT_LISTBOX,self.OnBehaviorSelected,blist)
			self.whales[n]['list'] = blist
	
	def OnBehaviorSelected(self,evt):
		gzt = evt.GetEventObject().GetClientData(evt.GetEventObject().GetSelections()[0])
		if gzt:
			cmd = 'gztime set '+ gzt
			self.gz.SendCommand(cmd)

class GeoSequenceApp(wx.App):

	def OnInit(self):
		f = GeoSequenceDialog(None)
		f.Show()
		self.SetTopWindow(f)
		return True

