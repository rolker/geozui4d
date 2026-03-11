#!/usr/bin/env python
# Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
# SPDX-License-Identifier: Apache-2.0


import numpy


class Kalman:
    def __init__(self,x0,P0,F,Q,H,R):
        '''
        x0 estimated initial state vector
        P0 error covariance matrix
        F state transition model
        Q is process noise covariance, or how much is the state expected to change
        H observation model, maps state space into observed space
        R observation noise covariance matrix
        '''

        self.x = x0 
        self.P = P0

        # constants
        self.F = F
        self.Q = Q
        self.H = H
        self.R = R


    def Add(self, z, Rk = None):
        # predict
        xp = numpy.dot(self.F,self.x)
        Pp = numpy.dot(numpy.dot(self.F,self.P),self.F.transpose())+self.Q

        if Rk is None:
            Rk = self.R

        # update
        y = z - numpy.dot(self.H,xp)
        S = numpy.dot(numpy.dot(self.H,Pp),self.H.transpose())+Rk
        self.K = numpy.dot(numpy.dot(Pp,self.H.transpose()),numpy.linalg.inv(S))
        xu = xp + numpy.dot(self.K,y)
        Pu = numpy.dot((numpy.identity(Pp.shape[0]) - numpy.dot(self.K,self.H)),Pp)

        self.x = xu
        self.P = Pu

        return self.x

class SimpleKalman(Kalman):
    def __init__(self,x,v,vz,vx=0):
        '''
        x is estimated starting position
        v is estimated starting velocity
        vz is variance of the noise of the position measurement
        vx is variance of the tracked process, or how much do we expecte the state to change
        '''

        x0 = numpy.array([[x],[v]])  # state vector
        P0 = numpy.array([[vz,0.0],[0.0,vz]]) # error covariance matrix
        F = numpy.array([[1.0,1.0],[0.0,1.0]]) # state transition model 
        Q = numpy.array([[vx,0.0],[0.0,vx]])
        H = numpy.array([[1.0,0.0]])
        R = numpy.array([[vz]])
        Kalman.__init__(self,x0,P0,F,Q,H,R)

    def Add(self,value,var=None):
        Rk = None
        if var is not None:
            Rk = numpy.array([[var]])
        return Kalman.Add(self, numpy.array([[value]]),Rk)[0,0]
