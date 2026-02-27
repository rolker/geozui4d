#!/usr/bin/env/ python

import xml.dom.minidom
import xml.xpath

class ItemBase:
	def __init__(self):
		self.attributes = {}

	def Read(self,element):
		# start with the standard strings
		for s in self.StringAttributes():
			if xml.xpath.Evaluate(''.join(('boolean(@',s,')')),element):
				self.attributes[s] = xml.xpath.Evaluate(''.join(('string(@',s,')')),element)

	def PrettyString(self,level=0,tab='  '):
		ret = ''
		for s in self.StringAttributes():
			try:
				ret = ''.join((ret,' ',s,': ',self.attributes[s]))
			except KeyError:
				pass
		ret = ''.join((ret,'\n'))
		return ret
