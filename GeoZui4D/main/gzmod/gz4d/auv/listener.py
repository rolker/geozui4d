#!/usr/bin/python
# gz
# auvListener
# Listens for RT AUV data from the net

import sys


import libxml2
import socket

import math

import time

class auvListener:
	def __init__(self):
		
		self.ships = dict()
		self.auvs = dict()
		
		self.xmlpacket = ""
		
		# we don't have a mapping of ship to AUV, so for now the first ships
		# id is saves in shipkey, and is used as the gz rt source, and to calculate
		# auv positions
		self.haveshipkey = False
		
		self.gzs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.gzs.connect(("127.0.0.1",1974))
		m = self.gzs.recv(2048)
		print "Connection established..."
		print m
		
		result = self.SendCommand("gz projection init")
		if result[0] == 1:
			print "Error initializing gz projections:"
			print result[1]
			
		
		self.logopen = False

		# open udp port
		

		
		self.connected = False
		
	def loop(self):
		if self.connected:
			try:
				b = self.s.recv(1024)
			except socket.timeout:
				time.sleep(0.25)
				return
			self.AddToPacket(b)
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
			if l == "":
				self.infile.close()
				self.logopen = False
				self.listenUDPBtn.Enable()
				self.logGauge.Hide()

			
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
			if v["needsgzupdate"] and v["hasgzobj"] and v.has_key("e") and v.has_key("n") and v.has_key("head"):
				gzcmd = "gz tellObject Object." + v["name"] + " setPositionOrientationTime " + str(v["e"]) + " " + str(v["n"]) + " 0 " + str(v["head"]) + " 0 0 " + str(v["time"])
				ret = self.SendCommand(gzcmd)
				if not k == self.shipkey:
					gzcmd = "gz tellObject Object." + v["name"] + " setPosition " + str(v["e"]) + " " + str(v["n"]) + " 0"
					self.SendCommand(gzcmd)
				v["needsgzupdate"] = False
			
		for k,v in self.auvs.iteritems():
			if v["needsgzupdate"] and v["hasgzobj"] and v.has_key("e") and v.has_key("n") and v.has_key("depth"):
				gzcmd = "gz tellObject Object." + v["name"] + " setPositionOrientationTime " + str(v["e"]) + " " + str(v["n"]) + " " + str(-v["depth"]) + " 0 0 0 " + str(v["time"])
				ret = self.SendCommand(gzcmd)
				gzcmd = "gz tellObject Object." + v["name"] + " setPosition " + str(v["e"]) + " " + str(v["n"]) + " " + str(-v["depth"])
				ret = self.SendCommand(gzcmd)
				v["needsgzupdate"] = False

		
	def OpenUDP(self,port):
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
		
	def LoadLog(self,lfn):
		self.xmlmsg = str("")
		self.linecount = 0
		try:
			self.infile = file(lfn,"rU")
		except IOError,details:
			print "Error opening " + lfn + ": " + details[1]
			return
		self.logopen = True
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
						if utmzone[0] == 0:
							projreq = "gz projection apply " + utmzone[1] + " " + str(lon[0]) + " " + str(lat[0])
							utm = self.SendCommand(projreq)
							if utm[0] == 0:
								self.ships[shipid]["e"] = float(utm[1].split()[0])
								self.ships[shipid]["n"] = float(utm[1].split()[1])
								self.ships[shipid]["needsgzupdate"] = True
								if not self.ships[shipid]["hasgzobj"]:
									objchkreq = "gz posable type Object."+self.ships[shipid]["name"]
									objchk = self.SendCommand(objchkreq)
									if objchk[0] == 1:   #error, so prob dosen't exists
										gzcmd = "gz newobject BasicVessel " + self.ships[shipid]["name"] + " " + str(self.ships[shipid]["e"]) + " " + str(self.ships[shipid]["n"]) + " 0"
										result = self.SendCommand(gzcmd)
										if result[0] == 0:
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
								if objchk[0] == 1:   #error, so prob dosen't exists
									gzcmd = "gz newobject BasicVessel " + self.auvs[auvid]["name"] + " " + str(self.auvs[auvid]["e"]) + " " + str(self.auvs[auvid]["n"]) + " 0"
									result = self.SendCommand(gzcmd)
									if result[0] == 0:
										self.auvs[auvid]["hasgzobj"] = True
										gzcmd = "gz tellObject " + self.auvs[auvid]["name"] + " path on"
										result = self.SendCommand(gzcmd)
								else:
									self.auvs[auvid]["hasgzobj"] = True
				if child.name == "telemetry":
					self.telemetryPacketLabel.SetLabel(msg)
				child = child.get_next()
			
		doc.freeDoc()
			
	def OpenShipModel(self,ship,model):
		gzcmd = "gz tellObject " + self.ships[ship]["name"] + " loadobject " + model
		result = self.SendCommand(gzcmd)
		gzcmd = "gz posable set " + result[1] + " h " + str(self.ships[k]["head"])
		result = self.SendCommand(gzcmd)
		
	def OpenAuvModel(self,auv,model):
		gzcmd = "gz tellObject " + self.auvs[auv]["name"] + " loadobject " + model
		result = self.SendCommand(gzcmd)
		#gzcmd = "gz posable set " + result[1] + " h " + str(self.auvs[k]["head"])
		#result = self.SendCommand(gzcmd)
		
	def SendCommand(self,cmd):
		self.gzs.send(str(cmd))
		res = self.gzs.recv(2048)
		result = [0,""]
		result[0] = int(res.splitlines()[0])
		result[1] = res.split("\n",1)[1].strip()
		return result
		

if __name__ == "__main__":
	a = auvListener()
	if len(sys.argv) != 2:
		print "Usage: auvListener port"
		sys.exit(1)
	a.OpenUDP(int(sys.argv[1]))
	while 1:
		a.loop()
