/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include <math.h>
#include <iostream>
#include "CubicSp.h"
    
using namespace std;

CubicSp::CubicSp(float t[], float y[], int n)
{
// Cubic spline from numerical recipes in C
// The indexing has been converted to 0..N-1

	int i,k;
	float p,qn,sig,un,*u;
	float yp1, ypn;
	
	yp1 = 0.0; ypn = 0.0;
	
	_n = n;
    
	_d2 = new float[n]; //allocate second derivative vector

	u = new float[_n];
	
	if (yp1 > 0.99e30) _d2[0]=u[0]=0.0;
	else 
	{
		_d2[0] = -0.5f;
		u[0]=(float)((3.0/(t[1]-t[0]))*((y[1]-y[0])/(t[1]-t[0])-yp1));
	}
	
	for (i=1;i<n-1;i++) 
	{
		sig=(t[i]-t[i-1])/(t[i+1]-t[i-1]);
		p=(float)(sig*_d2[i-1]+2.0);
		_d2[i]=(float)((sig-1.0)/p);
		u[i]=(y[i+1]-y[i])/(t[i+1]-t[i]) - (y[i]-y[i-1])/(t[i]-t[i-1]);
		u[i]=(float)((6.0*u[i]/(t[i+1]-t[i-1])-sig*u[i-1])/p);
	}
	
	if (ypn > 0.99e30) qn=un=0.0;
	else 
	{
		qn=0.5f;
		un=(float)((3.0/(t[n-1]-t[n-2]))*(ypn-(y[n-1]-y[n-2])/(t[n-1]-t[n-2])));
	}
	
	_d2[n-1]=(float)((un-qn*u[n-2])/(qn*_d2[n-2]+1.0));
	
	for (k=n-2;k>=0;k--)
	{
		_d2[k]=_d2[k]*_d2[k+1]+u[k];
		
	}	
	delete [] u;
}

CubicSp::~CubicSp()
{
	delete [] _d2;
}

void CubicSp::resample(float t[], float y[], float tt[], float yy[], int m)
// Input t and y vectors of length n
// output tt and yy vectors of length m
// typically m >> n
{
    float start, end, step, sample;
    int i;
    
    start = t[0]; end = t[_n-1];
    step = (end - start)/float(m-1);
    
    //spline( t, y,  n);
    for (i=0;i<m;++i)
    {
	sample = start + float(i)*step;
	tt[i] = sample;
	yy[i] = splint( t,  y, sample);
	
    }
	cerr << "RES\n";
}




float CubicSp:: splint(float xa[], float ya[],  float x)
// This interpolates the spline that has already been defined.
{	
	int klo,khi,k;
	float h,b,a;

	klo = 0;
	khi = _n-1;
	while (khi-klo > 1) {
		k=(khi+klo) >> 1;
		if (xa[k] > x) khi=k;
		else klo=k;
	}
	h=xa[khi]-xa[klo];
	if (h == 0.0) cerr << "Bad xa input to routine splint \n";
	a=(xa[khi]-x)/h;
	b=(x-xa[klo])/h;
	
	return (float)(a*ya[klo]+b*ya[khi]+((a*a*a-a)*_d2[klo]+(b*b*b-b)*_d2[khi])*(h*h)/6.0);
}
