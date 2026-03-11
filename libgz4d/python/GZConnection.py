#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import socket

class GZConnection:
    def __init__(self):
        self.socket = None

    def IsConnected(self):
        return self.socket is not None

    def Send(self,msg):
        if self.socket is not None:
            try:
                self.socket.send(msg)
                return self.socket.recv(2048).split('\n',1)
            except socket.error:
                self.socket = None

    def Connect(self,address=None):
        if address == None:
            address = ('localhost',1974)

        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect(address)
            ret = self.socket.recv(2048)
        except socket.error:
            self.socket = None

