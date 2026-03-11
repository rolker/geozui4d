#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0

'''
Direct connection to a GZ Object.

Listens for a connection from a GZ object and provides
asyncronous communication directly with an object
instance without needing to go through tcl
'''

import socket
import Queue
import threading

from GzError import GzError

class LostConnectionError(GzError):
    pass

class UnknownObjectError(GzError):
    pass

class ObjectConnection:
    def __init__(self,inport,outport,callback):
    #def __init__(self,inport,outport):
        '''
        Allows asyncronous connection to objects in GZ. Data is recieved
        and passed to a callback function with the data being the sole
        argument. The sendData method is uded to send data to the object.
        @param inport: Port to listen to.
        @param outport: Port to send to.
        @param callback: Function to call when data is recieved.
        '''
        self.server = ObjectConnectionServer(inport,outport,callback)
        #self.server = ObjectConnectionServer(inport,outport)
    
    def __del__(self):
        self.server.abort.set()
        
    def sendData(self,data):
        '''
        Send a message to the GeoZui object.
        @param data: Data message to send
        '''
        self.server.send(data)

    #def getData(self):
        #return self.server.receive()

class ObjectConnectionServer:
    def __init__(self,inport,outport,callback):
    #def __init__(self,inport,outport):
        self.abort = threading.Event()
        self.closed = threading.Event()
        self.outbox = Queue.Queue()
        #self.inbox = Queue.Queue()

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.bind(('127.0.0.1',inport))
        except socket.error:
            s.close()
            self.closed.set()
        else:
            s.settimeout(0.02)
        
            threading.Thread(target=self.processConnections,args=(s,outport,callback),name=' '.join(('ObjectConnection - port:',str(inport)))).start()
            #threading.Thread(target=self.processConnections,args=(s,outport),name=' '.join(('ObjectConnection - port:',str(inport)))).start()



            
    def __del__(self):
        self.abort.set()
        self.closed.wait()
        
    def processConnections(self,s,outport,callback):
    #def processConnections(self,s,outport):
        while 1:
            try:
                indata = s.recv(2048)
            except socket.timeout:
                pass
            else:
                callback(indata)
#                self.inbox.put(indata)
            try:
                outdata = self.outbox.get_nowait()
            except Queue.Empty:
                pass
            else:
                s.sendto(outdata,('127.0.0.1',outport))
            if self.abort.isSet():
                break;
        print 'Server thread shutting down'
        s.close()
        self.closed.set()
        
    def send(self,data):
        self.outbox.put(data)

    #def receive(self):
        #try:
            #return self.inbox.get_nowait()
        #except Queue.Empty:
            #return None
