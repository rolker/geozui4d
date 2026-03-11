#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


class OrderedDict(dict):
	'''
	Dictionary that keeps track of the order of inserted items.
	(based on ActiveState Python recipe 107747)
	'''
	def __init__(self, i = None):
		self.orderedKeys = []
		if i:
			dict.__init__(self, i)
		else:
			dict.__init__(self)
		
	def __delitem__(self, key):
		dict.__delitem__(self,key)
		self.orderedKeys.remove(key)
		
	def __setitem__(self, key, item):
		dict.__setitem__(self, key, item)
		if key not in self.orderedKeys:
			self.orderedKeys.append(key)
			
	def clear(self):
		dict.clear(self)
		self.orderedKeys = []
		
	def copy(self):
		ret = dict.copy(self)
		ret.orderedKeys = self.orderedKeys[:]
		return ret
	
	def items(self):
		return zip(self.orderedKeys, self.values())
	
	def keys(self):
		return self.orderedKeys
		
	def popitem(self):
		try:
			key = self.orderedKeys[-1]
		except IndexError:
			raise KeyError('ItemCollection is empty')
		
		val = self[key]
		del self[key]
		
	def setdefault(self, key, failobj = None):
		dict.setdefault(self,key,failobj)
		if key not in self.orderedKeys:
			self.orderedKeys.append(key)
			
	def update(self, d):
		dict.update(self, d)
		for key in d.keys():
			if key not in self.orderedKeys:
				self.orderedKeys.append(key)
				
	def values(self):
		return map(self.get, self.orderedKeys)
