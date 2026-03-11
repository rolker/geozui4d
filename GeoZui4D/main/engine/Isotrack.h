// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef ISOTRACK_H
#define ISOTRACK_H

#ifdef WIN32

#include<windows.h>

#endif

class Isotrack
{
public:
	void PrintValues();
	void LaunchPositionThread();
	int GetStatus();
	void Calibrate();
	void GetPosition(int sensor, double * X, double * Y, double * Z,
					 double * Q1, double * Q2, double * Q3, double * Q4);
	void SetQuaternionFastrack();
	void SetQuaternionIsotrack();
	int Initialize(int d=0);
	Isotrack();
#ifdef WIN32
	static long WINAPI PositionThread(long dummy);
#endif
protected:
#ifdef WIN32
	static HANDLE hComm;
	HANDLE hPositionThread;
#endif
	static float x[4];
	static float y[4];
	static float z[4];
	static float q1[4];
	static float q2[4];
	static float q3[4];
	static float q4[4];

#ifdef WIN32
	static CRITICAL_SECTION * CriticalSection;
#endif
};


#endif
