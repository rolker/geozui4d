#!/usr/bin/python
# gz
# auvManager
# Manages RT AUV data from the net or replay of logged data

import wx
import sys

import libxml2
import socket
import datetime

import math

import time

import gz4d

class auvManagerFrame(wx.Frame):
	def __init__(self, parent, title):
		wx.Frame.__init__(self,parent,-1,title)
		
		
		self.address = sys.argv[1]
		self.appdir = sys.argv[2]
		self.datadir = sys.argv[3]

		self.gz = gz4d.Connection()
		self.gz.Connect()
		
		self.ships = dict()
		self.auvs = dict()
		self.shipguis = dict()
		self.auvguis = dict()
		
		self.xmlpacket = ""
		
		# we don't have a mapping of ship to AUV, so for now the first ships
		# id is saves in shipkey, and is used as the gz rt source, and to calculate
		# auv positions
		self.haveshipkey = False
		
		result = self.SendCommand("gz projection init")
		if result.RetVal == 1:
			print "Error initializing gz projections:"
			print result.RetStr
			
		
		# open log button
		
		self.openlogBtn = wx.Button(self,label="Open log")
		self.logLabel = wx.StaticText(self)
		self.logLabel.SetLabel("(none selected)")
		self.logGauge = wx.Gauge(self)
		self.logGauge.Hide()
		
		self.logopen = False

		self.Bind(wx.EVT_BUTTON, self.OnLoadLog,self.openlogBtn)
		
		# open udp port
		
		self.portTextCtrl = wx.TextCtrl(self)
		self.portTextCtrl.SetValue("10500")
		
		self.listenUDPBtn = wx.Button(self,label="listen on UDP port")
		
		self.connectedLabel = wx.StaticText(self)
		self.connectedLabel.SetLabel("Not connected.")
		
		self.Bind(wx.EVT_BUTTON, self.OnListenUDP,self.listenUDPBtn)
		self.Bind(wx.EVT_IDLE,self.OnIdle)
		self.connected = False
		
		# load related gz data
		
		self.reldataLabel = wx.StaticText(self,label="Related data:")
		self.reldatafnText = wx.TextCtrl(self,value="/home/roland/work/SeaBed/Oct2005/santa_lucia.gzx")
		self.reldatafselBtn = wx.Button(self,label="Select...")
		self.reldatagzBtn = wx.Button(self,label="Open in GZ")
		
		self.Bind(wx.EVT_BUTTON, self.OnSelectData,self.reldatafselBtn)
		self.Bind(wx.EVT_BUTTON, self.OnSendGZDataCmd,self.reldatagzBtn)
		
		################
		#layout widgets#
		################
		
		sizer = wx.BoxSizer(wx.VERTICAL)
		
		logsizer = wx.BoxSizer(wx.HORIZONTAL)
		
		logsizer.Add(self.openlogBtn,0,wx.ALIGN_CENTER_VERTICAL)
		logsizer.Add(self.logLabel,0,wx.ALIGN_CENTER_VERTICAL)
		logsizer.Add(self.logGauge,1,wx.ALIGN_CENTER_VERTICAL|wx.EXPAND)
		
		sizer.Add(logsizer)
		
		udpsizer = wx.BoxSizer(wx.HORIZONTAL)
		
		udpsizer.Add(self.portTextCtrl,0,wx.ALIGN_CENTER_VERTICAL)
		udpsizer.Add(self.listenUDPBtn,0,wx.ALIGN_CENTER_VERTICAL)
		udpsizer.Add(self.connectedLabel,1,wx.ALIGN_CENTER_VERTICAL)
		
		sizer.Add(udpsizer)
		
		datasizer = wx.BoxSizer(wx.HORIZONTAL)
		
		datasizer.Add(self.reldataLabel,0,wx.ALIGN_CENTER_VERTICAL)
		datasizer.Add(self.reldatafnText,1,wx.ALIGN_CENTER_VERTICAL|wx.EXPAND)
		datasizer.Add(self.reldatafselBtn,0,wx.ALIGN_CENTER_VERTICAL)
		datasizer.Add(self.reldatagzBtn,0,wx.ALIGN_CENTER_VERTICAL)
		
		sizer.Add(datasizer,0,wx.EXPAND)
		
		self.shipsizer = wx.GridBagSizer(5,5)
		self.shipsizer.Add(wx.StaticText(self,label="Name",style=wx.RAISED_BORDER),(0,0))
		self.shipsizer.Add(wx.StaticText(self,label="Latitude",style=wx.SIMPLE_BORDER),(0,1))
		self.shipsizer.Add(wx.StaticText(self,label="Longitude",style=wx.SIMPLE_BORDER),(0,2))
		self.shipsizer.Add(wx.StaticText(self,label="Heading",style=wx.SIMPLE_BORDER),(0,3))
		self.shipsizer.Add(wx.StaticText(self,label="Time",style=wx.SIMPLE_BORDER),(0,4))
		self.shipsizer.Add(wx.StaticText(self,label="e",style=wx.SIMPLE_BORDER),(0,5))
		self.shipsizer.Add(wx.StaticText(self,label="n",style=wx.SIMPLE_BORDER),(0,6))
		self.shipsizer.Add(wx.StaticText(self,label="Model",style=wx.SIMPLE_BORDER),(0,7))
		self.shipsizer.Add(wx.StaticText(self,label="Select",style=wx.SIMPLE_BORDER),(0,8))
		self.shipsizer.Add(wx.StaticText(self,label="Open in GZ",style=wx.SIMPLE_BORDER),(0,9))
		
		self.shipsizer.AddGrowableCol(7,1)
		
		self.shipsizernextrow = 1
		
		sizer.Add(self.shipsizer,flag=wx.EXPAND)
		
		self.auvsizer = wx.GridBagSizer(5,5)
		self.auvsizer.Add(wx.StaticText(self,label="Name",style=wx.SIMPLE_BORDER),(0,0))
		self.auvsizer.Add(wx.StaticText(self,label="Range",style=wx.SIMPLE_BORDER),(0,1))
		self.auvsizer.Add(wx.StaticText(self,label="Bearing",style=wx.SIMPLE_BORDER),(0,2))
		self.auvsizer.Add(wx.StaticText(self,label="Azimuth",style=wx.SIMPLE_BORDER),(0,3))
		self.auvsizer.Add(wx.StaticText(self,label="Time",style=wx.SIMPLE_BORDER),(0,4))
		self.auvsizer.Add(wx.StaticText(self,label="e",style=wx.SIMPLE_BORDER),(0,5))
		self.auvsizer.Add(wx.StaticText(self,label="n",style=wx.SIMPLE_BORDER),(0,6))
		self.auvsizer.Add(wx.StaticText(self,label="depth",style=wx.SIMPLE_BORDER),(0,7))
		self.auvsizer.Add(wx.StaticText(self,label="Model",style=wx.SIMPLE_BORDER),(0,8))
		self.auvsizer.Add(wx.StaticText(self,label="Select",style=wx.SIMPLE_BORDER),(0,9))
		self.auvsizer.Add(wx.StaticText(self,label="Open in GZ",style=wx.SIMPLE_BORDER),(0,10))
		
		self.auvsizer.AddGrowableCol(8,1)

		self.auvsizernextrow = 1
		
		sizer.Add(self.auvsizer,flag=wx.EXPAND)
		
		self.shipPacketLabel = wx.StaticText(self)
		self.trackingPacketLabel = wx.StaticText(self)
		self.telemetryPacketLabel = wx.StaticText(self)
		
		sizer.Add(self.shipPacketLabel)
		sizer.Add(self.trackingPacketLabel)
		sizer.Add(self.telemetryPacketLabel)
		
		self.SetSizer(sizer)
		self.Layout()
		
		
	def	OnSelectData(self,evt):
		print "OnSelectData"
		
	def OnSendGZDataCmd(self,evt):
		 gzcmd = "gz loadobject " + self.reldatafnText.GetValue()
		 print gzcmd
		 self.SendCommand(gzcmd)
	
	def OnIdle(self,evt):
		if self.connected:
			try:
				b = self.s.recv(1024)
			except socket.timeout:
				time.sleep(0.25)
				return
			self.AddToPacket(b)
			evt.RequestMore()
		if self.logopen:
			c = 0
			l = self.infile.readline()
			while  l:
				self.linecount += 1
				if l[0:5] == "<auv>":
					self.xmlmsg = l
				else:
					self.xmlmsg = self.xmlmsg + l
				if l[0:6] == "</auv>":
					self.ProcessMsg(self.xmlmsg)
					self.UpdateInfo()
				c += 1
				if c > 100:
					break
				else:
					l = self.infile.readline()
			self.logGauge.SetValue(self.linecount)
			if l == "":
				self.infile.close()
				self.logopen = False
				self.listenUDPBtn.Enable()
				self.logGauge.Hide()
			else:
				evt.RequestMore()

			
	def AddToPacket(self,b):
		lines = b.splitlines()
		for l in lines:
			if self.xmlpacket == "":
				if l[0:5] == "<auv>":
					self.xmlpacket = l
			else:
				if l and l.lstrip() and l.lstrip()[0] == "<":
					self.xmlpacket += "\n" + l
				else:
					self.xmlpacket += l
				if l[0:6] == "</auv>":
					self.ProcessMsg(self.xmlpacket)
					self.xmlpacket = ""
		self.UpdateInfo()
		
		
	def UpdateInfo(self):

		for k,v in self.ships.iteritems():
			if not self.shipguis.has_key(k):
				self.shipguis[k] = dict()
				self.shipguis[k]["name"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["name"],(self.shipsizernextrow,0))
				
				self.shipguis[k]["lat"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["lat"],(self.shipsizernextrow,1))
				
				self.shipguis[k]["lon"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["lon"],(self.shipsizernextrow,2))
				
				self.shipguis[k]["head"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["head"],(self.shipsizernextrow,3))

				self.shipguis[k]["time"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["time"],(self.shipsizernextrow,4))
				
				self.shipguis[k]["e"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["e"],(self.shipsizernextrow,5))
				
				self.shipguis[k]["n"] = wx.StaticText(self)
				self.shipsizer.Add(self.shipguis[k]["n"],(self.shipsizernextrow,6))
				
				self.shipguis[k]["modelfn"] = wx.TextCtrl(self,value="/home/roland/work/SeaBed/ship.dxf")
				self.shipsizer.Add(self.shipguis[k]["modelfn"],(self.shipsizernextrow,7),flag=wx.EXPAND)
				
				self.shipguis[k]["modelsel"] = wx.Button(self,label="...",name=k)
				self.shipsizer.Add(self.shipguis[k]["modelsel"],(self.shipsizernextrow,8))
				self.Bind(wx.EVT_BUTTON, self.OnSelectShipModel,self.shipguis[k]["modelsel"])
				
				self.shipguis[k]["modelopen"] = wx.Button(self,label="Open",name=k)
				self.shipsizer.Add(self.shipguis[k]["modelopen"],(self.shipsizernextrow,9))
				self.Bind(wx.EVT_BUTTON, self.OnOpenShipModel,self.shipguis[k]["modelopen"])

				self.shipsizernextrow += 1
				self.Layout()
			
			self.shipguis[k]["name"].SetLabel(v["name"])
			if v.has_key("lat"):
				self.shipguis[k]["lat"].SetLabel(v["lat"])
			if v.has_key("lon"):
				self.shipguis[k]["lon"].SetLabel(v["lon"])
			if v.has_key("head"):
				self.shipguis[k]["head"].SetLabel(str(v["head"]))
			self.shipguis[k]["time"].SetLabel(datetime.datetime.utcfromtimestamp(float(v["time"])).isoformat())
			if v.has_key("e"):
				self.shipguis[k]["e"].SetLabel(str(v["e"]))
			if v.has_key("n"):
				self.shipguis[k]["n"].SetLabel(str(v["n"]))
			if v["needsgzupdate"] and v["hasgzobj"] and v.has_key("e") and v.has_key("n") and v.has_key("head"):
				gzcmd = "gz tellObject Object." + v["name"] + " setPositionOrientationTime " + str(v["e"]) + " " + str(v["n"]) + " 0 " + str(v["head"]) + " 0 0 " + str(v["time"])
				ret = self.SendCommand(gzcmd)
				if not k == self.shipkey:
					gzcmd = "gz tellObject Object." + v["name"] + " setPosition " + str(v["e"]) + " " + str(v["n"]) + " 0"
					ret = self.SendCommand(gzcmd)
				v["needsgzupdate"] = False
			self.Layout()
			
		for k,v in self.auvs.iteritems():
			if not self.auvguis.has_key(k):
				self.auvguis[k] = dict()
				self.auvguis[k]["name"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["name"],(self.auvsizernextrow,0))
				
				self.auvguis[k]["range"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["range"],(self.auvsizernextrow,1))
				
				self.auvguis[k]["bearing"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["bearing"],(self.auvsizernextrow,2))
				
				self.auvguis[k]["azimuth"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["azimuth"],(self.auvsizernextrow,3))
				
				self.auvguis[k]["time"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["time"],(self.auvsizernextrow,4))
				
				self.auvguis[k]["e"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["e"],(self.auvsizernextrow,5))
				
				self.auvguis[k]["n"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["n"],(self.auvsizernextrow,6))
				
				self.auvguis[k]["depth"] = wx.StaticText(self)
				self.auvsizer.Add(self.auvguis[k]["depth"],(self.auvsizernextrow,7))
				
				self.auvguis[k]["modelfn"] = wx.TextCtrl(self,value="/home/roland/work/SeaBed/seabed_2.dxf")
				self.auvsizer.Add(self.auvguis[k]["modelfn"],(self.auvsizernextrow,8),flag=wx.EXPAND)
				
				self.auvguis[k]["modelsel"] = wx.Button(self,label="...",name=k)
				self.auvsizer.Add(self.auvguis[k]["modelsel"],(self.auvsizernextrow,9))
				self.Bind(wx.EVT_BUTTON, self.OnSelectAuvModel,self.auvguis[k]["modelsel"])
				
				self.auvguis[k]["modelopen"] = wx.Button(self,label="Open",name=k)
				self.auvsizer.Add(self.auvguis[k]["modelopen"],(self.auvsizernextrow,10))
				self.Bind(wx.EVT_BUTTON, self.OnOpenAuvModel,self.auvguis[k]["modelopen"])
				
				self.auvsizernextrow += 1
				self.Layout()
			
			self.auvguis[k]["name"].SetLabel(v["name"])
			if v.has_key("range"):
				self.auvguis[k]["range"].SetLabel(str(v["range"]))
			if v.has_key("bearing"):
				self.auvguis[k]["bearing"].SetLabel(str(v["bearing"]))
			if v.has_key("azimuth"):
				self.auvguis[k]["azimuth"].SetLabel(str(v["azimuth"]))
			self.auvguis[k]["time"].SetLabel(datetime.datetime.utcfromtimestamp(float(v["time"])).isoformat())
			if v.has_key("e"):
				self.auvguis[k]["e"].SetLabel(str(v["e"]))
			if v.has_key("n"):
				self.auvguis[k]["n"].SetLabel(str(v["n"]))
			if v.has_key("depth"):
				self.auvguis[k]["depth"].SetLabel(str(v["depth"]))
			if v["needsgzupdate"] and v["hasgzobj"] and v.has_key("e") and v.has_key("n") and v.has_key("depth"):
				gzcmd = "gz tellObject Object." + v["name"] + " setPositionOrientationTime " + str(v["e"]) + " " + str(v["n"]) + " " + str(-v["depth"]) + " 0 0 0 " + str(v["time"])
				ret = self.SendCommand(gzcmd)
				gzcmd = "gz tellObject Object." + v["name"] + " setPosition " + str(v["e"]) + " " + str(v["n"]) + " " + str(-v["depth"])
				ret = self.SendCommand(gzcmd)
				v["needsgzupdate"] = False

		
	def OnListenUDP(self,evt):
		port = int(self.portTextCtrl.GetValue())
		print "Opening port " + str(port) + " for listening"
		self.s = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
		self.s.settimeout(0.25)
		try:
			self.s.bind(("",port))
		except socket.error, inst:
			if inst.args[0] == 98: #port in use
				print "Port " + str(port) + " is already in use."
				self.connected = False
				return
			else:
				raise
		self.connected = True
		self.connectedLabel.SetLabel("Connected, listening on UDP port "+str(port)+".")
		self.openlogBtn.Disable()
		
		
	def OnLoadLog(self,evt):
		fdlg = wx.FileDialog(self,style=wx.OPEN)
		fdlg.SetDirectory(self.datadir)
		ret = fdlg.ShowModal()
		if ret == wx.ID_OK:
			self.LoadLog(fdlg.GetPath())
			

	def LoadLog(self,lfn):
		self.xmlmsg = str("")
		self.linecount = 0
		try:
			self.infile = file(lfn,"rU")
		except IOError,details:
			print "Error opening " + lfn + ": " + details[1]
			return
		for l in self.infile:
			self.linecount += 1
		self.infile.seek(0)
		self.logopen = True
		self.listenUDPBtn.Disable()
		self.logLabel.SetLabel(lfn)
		self.logGauge.SetRange(self.linecount)
		self.logGauge.Show()
		self.linecount = 0
		
	def ProcessMsg(self,msg):
		
		try:
			doc = libxml2.parseDoc(msg)
		except libxml2.parserError:
			print "Bad xml message:"
			print msg + "\n"
			return
		root = doc.children
		if root.name == "auv":
			child = root.children
			while child:
				if child.name == "ship":
					self.shipPacketLabel.SetLabel(msg)
					shipname = child.prop("shipName")
					shipid = child.prop("shipID")
					subchild = child.children
					lat = [0.0,0]
					lon = [0.0,0]
					h   = [0.0,0]
					
					while subchild:
						if subchild.name == "time":
							time = subchild.getContent()
						if subchild.name == "latitude":
							lat[0] = subchild.getContent()
							lat[1] = 1
						if subchild.name == "longitude":
							lon[0] = subchild.getContent()
							lon[1] = 1
						if subchild.name == "heading":
							h[0] = subchild.getContent()
							h[1] = 1
						subchild = subchild.get_next()
					if self.ships.has_key(shipid):
						self.ships[shipid]["count"] += 1
					else:
						self.ships[shipid] = dict()
						self.ships[shipid]["name"] = shipname
						self.ships[shipid]["count"] = 1
						self.ships[shipid]["hasgzobj"] = False
						self.ships[shipid]["needsgzupdate"] = True
						if not self.haveshipkey:
							self.shipkey = shipid
							self.haveshipkey = True
							
					self.ships[shipid]["time"] = time
					if lat[1] == 1:
						self.ships[shipid]["lat"] = lat[0]
					if lon[1] == 1:
						self.ships[shipid]["lon"] = lon[0]
					if h[1] == 1:
						self.ships[shipid]["head"] = float(h[0])
						self.ships[shipid]["needsgzupdate"] = True
					# project to utm
					if lat[1] == 1 and lon[1] == 1:
						utmzonereq = "gz projection longToUTM " + lon[0]
						#utmzone = self.gz.SendCommand(utmzonereq)
						utmzone = self.SendCommand(utmzonereq)
						if utmzone.RetVal == 0:
							projreq = "gz projection apply " + utmzone.RetStr + " " + str(lon[0]) + " " + str(lat[0])
							utm = self.SendCommand(projreq)
							if utm.RetVal == 0:
								self.ships[shipid]["e"] = float(utm.RetStr.split()[0])
								self.ships[shipid]["n"] = float(utm.RetStr.split()[1])
								self.ships[shipid]["needsgzupdate"] = True
								if not self.ships[shipid]["hasgzobj"]:
									objchkreq = "gz posable type Object."+self.ships[shipid]["name"]
									objchk = self.SendCommand(objchkreq)
									if objchk.RetVal == 1:   #error, so prob dosen't exists
										gzcmd = "gz newobject BasicVessel " + self.ships[shipid]["name"] + " " + str(self.ships[shipid]["e"]) + " " + str(self.ships[shipid]["n"]) + " 0"
										result = self.SendCommand(gzcmd)
										if result.RetVal == 0:
											self.ships[shipid]["hasgzobj"] = True
										gzcmd = "gz tellObject " + self.ships[shipid]["name"] + " path on"
										result = self.SendCommand(gzcmd)
									else:
										self.ships[shipid]["hasgzobj"] = True
									if self.haveshipkey and self.shipkey == shipid:
										gzcmd = "gz tellObject Object." + self.ships[shipid]["name"] + " setRT"
										result = self.SendCommand(gzcmd)

								
				if child.name == "tracking":
					self.trackingPacketLabel.SetLabel(msg)
					auvname = child.prop("auvName")
					auvid = child.prop("auvID")
					subchild = child.children
					rng = [0.0,0]
					br  = [0.0,0]
					az  = [0.0,0]
					
					while subchild:
						if subchild.name == "time":
							time = subchild.getContent()
						if subchild.name == "range":
							rng[0] = float(subchild.getContent())
							rng[1] = 1
						if subchild.name == "bearing":
							br[0] = float(subchild.getContent())
							br[1] = 1
						if subchild.name == "azimuth":
							az[0] = float(subchild.getContent())
							az[1] = 1
						subchild = subchild.get_next()
					if self.auvs.has_key(auvid):
						self.auvs[auvid]["count"] += 1
					else:
						self.auvs[auvid] = dict()
						self.auvs[auvid]["name"] = auvname
						self.auvs[auvid]["count"] = 1
						self.auvs[auvid]["hasgzobj"] = False
						self.auvs[auvid]["needsgzupdate"] = True
					self.auvs[auvid]["time"] = time
					if rng[1] == 1:
						self.auvs[auvid]["range"] = rng[0]
					if br[1] == 1:
						self.auvs[auvid]["bearing"] = br[0]
					if az[1] == 1:
						self.auvs[auvid]["azimuth"] = az[0]
					# calculate position and depth
					if self.haveshipkey and az[1] == 1 and not az[0] == 0 and br[1] == 1 and rng[1] == 1:
						if self.ships[self.shipkey].has_key("e") and self.ships[self.shipkey].has_key("n"):
							#if we got here, we should have all the necessary components
							self.auvs[auvid]["depth"] = rng[0]*math.sin(math.radians(az[0]))
							self.auvs[auvid]["e"] = self.ships[self.shipkey]["e"] + rng[0]*math.sin(math.radians(self.ships[self.shipkey]["head"]+br[0]))
							self.auvs[auvid]["n"] = self.ships[self.shipkey]["n"] + rng[0]*math.cos(math.radians(self.ships[self.shipkey]["head"]+br[0]))
							self.auvs[auvid]["needsgzupdate"] = True
							if not self.auvs[auvid]["hasgzobj"]:
								objchkreq = "gz posable type Object."+self.auvs[auvid]["name"]
								objchk = self.SendCommand(objchkreq)
								if objchk.RetVal == 1:   #error, so prob dosen't exists
									gzcmd = "gz newobject BasicVessel " + self.auvs[auvid]["name"] + " " + str(self.auvs[auvid]["e"]) + " " + str(self.auvs[auvid]["n"]) + " 0"
									result = self.SendCommand(gzcmd)
									if result.RetVal == 0:
										self.auvs[auvid]["hasgzobj"] = True
										gzcmd = "gz tellObject " + self.auvs[auvid]["name"] + " path on"
										result = self.SendCommand(gzcmd)
								else:
									self.auvs[auvid]["hasgzobj"] = True
				if child.name == "telemetry":
					self.telemetryPacketLabel.SetLabel(msg)
				child = child.get_next()
			
		doc.freeDoc()
			
			
	def OnSelectShipModel(self,evt):
		k = evt.GetEventObject().GetName()
		fdlg = wx.FileDialog(self,style=wx.OPEN)
		fdlg.SetDirectory(self.datadir)
		ret = fdlg.ShowModal()
		if ret == wx.ID_OK:
			self.shipguis[k]["modelfn"].SetValue(fdlg.GetPath())
		
	def OnOpenShipModel(self,evt):
		k = evt.GetEventObject().GetName()
		gzcmd = "gz tellObject " + self.ships[k]["name"] + " loadobject " + self.shipguis[k]["modelfn"].GetValue()
		result = self.SendCommand(gzcmd)
		gzcmd = "gz posable set " + result.RetStr + " h " + str(self.ships[k]["head"])
		result = self.SendCommand(gzcmd)
		
	def OnSelectAuvModel(self,evt):
		k = evt.GetEventObject().GetName()
		fdlg = wx.FileDialog(self,style=wx.OPEN)
		fdlg.SetDirectory(self.datadir)
		ret = fdlg.ShowModal()
		if ret == wx.ID_OK:
			self.auvguis[k]["modelfn"].SetValue(fdlg.GetPath())
		
	def OnOpenAuvModel(self,evt):
		k = evt.GetEventObject().GetName()
		gzcmd = "gz tellObject " + self.auvs[k]["name"] + " loadobject " + self.auvguis[k]["modelfn"].GetValue()
		result = self.SendCommand(gzcmd)
		#gzcmd = "gz posable set " + result.RetStr + " h " + str(self.auvs[k]["head"])
		#result = self.SendCommand(gzcmd)
		
	def SendCommand(self,cmd):
		result = self.gz.SendCommand(str(cmd))
		return result
		

class auvManager(wx.App):

	def OnInit(self):
		f = auvManagerFrame(None,"AUV manager")
		f.Show()
		self.SetTopWindow(f)
		return True
		
if __name__ == "__main__":
	app = auvManager()
	app.MainLoop()

