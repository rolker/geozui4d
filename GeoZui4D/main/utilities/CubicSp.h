// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef CUBICSP_H
#define CUBICSP_H


class CubicSp
{
    public:
	CubicSp(float t[], float y[], int n);
	~CubicSp();

	void  resample(float t[], float y[], float xx[], float yy[], int m);
	
	private:
	float splint(float t[], float y[], float x);	
	float *_d2; // this stores the second derivative
	int _n;  // number of control points


};

#endif

/*


void  CubicSp(float t[], float y[], int n);
    This constructs the natural cubic through the t and y input vectors
    n is the number of knots.    
    The start gradient = the end gradient
    
float splint(float xa[], float ya[], float y2a[], float t);

    This evaluates the spline at point t
    

void  resample(float t[], float y[], float tt[], float yy[], int m);

    resamples into the vectors tt and yy of length m

*/
