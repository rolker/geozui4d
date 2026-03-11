// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ForwardCubicSpline.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class handles cubic-spline resampling of arrays of floats.  It
 *	assumes evenly spaced values in the input (t) domain to increase its
 *	efficiency (y varies).
 *
 *	Converted by mdp 4/10/2003 from CubicSpline and InverseCubicSpline, which
 *	in turn were adapted from Colin's CubicSp class, which
 *	in turn was an adaptation of Numerical Recipes in C's implementation.
 */
/*---------------------------------------------------------------------------*/
#include "utilities/ForwardCubicSpline.h"
#include "widgets/Sv_Standards.h"
#include <math.h>
    
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ForwardCubicSpline::ForwardCubicSpline(const float *points, int n)
{
	delta = NULL;
	_d2 = NULL;
	_n = -1;
	setControlPoints(points, n);
}

/*---------------------------------------------------------------------------*/
ForwardCubicSpline::~ForwardCubicSpline()
{
	delete [] delta;
	delete [] _d2;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ForwardCubicSpline::setControlPoints(const float *points, int n)
{
	int oldN, i;
	float *currentDelta;
	const float *currentPoint, *previousPoint;

	oldN = _n;
	_n = n;
	controlPoints = points;

	if (!points)
		return;

	//create derivative arrays.
	if (delta && n != oldN)
	{
		delete [] delta;
		delete [] _d2;
		delta = NULL;
	}
	if (!delta)
	{
		delta = new float[n];
		_d2 = new float[n];
	}

	//initialize deltas.
	delta[0] = 0.0f;
	currentDelta = &delta[1];
	previousPoint = &controlPoints[0];
	currentPoint = &controlPoints[1];
	for (i = 1; i < n; i++)
		*currentDelta++ = *currentPoint++ - *previousPoint++;

	recalculationRequired = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void ForwardCubicSpline::pointUpdated(int index)
{
	if (index > 0)
		delta[index] = controlPoints[index] - controlPoints[index - 1];
	if (index < _n)
		delta[index + 1] = controlPoints[index + 1] - controlPoints[index];
	recalculationRequired = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void ForwardCubicSpline::recalculate2ndDerivative()
{
	int k, end;
	float p;
	float *u;
	float *currentDelta, *previousDelta, *currentU, *previousU;
	float *nextD2, *d2Val;
	
	u = new float[_n];
	
	previousU = currentU = u;
	currentDelta = &delta[1];
	d2Val = _d2;

	*d2Val = -0.5f;
	*currentU++ = 3.0f * (*currentDelta++);
	
	//work forwards
	previousDelta = &delta[1];
	end = _n - 1;
	for (k = 1; k < end; k++) 
	{
		p = *d2Val++ + 4.0f;
		*d2Val= (-1.0f) / p;
		*currentU++ = (6.0f * (*currentDelta++ - *previousDelta++) - (*previousU++)) / p;
	}
	
	p = *d2Val++ + 2.0f;

	*d2Val = (-6.0f * (*previousDelta++) - (*previousU++)) / p;
	
	//now work backwards
	nextD2 = &_d2[_n - 1];
	d2Val = &_d2[_n - 2];
	currentU = &(u[_n-2]);
	for (k= _n-2; k >= 0; k--)
	{
		*d2Val = *d2Val * (*nextD2--) + *currentU--;
		d2Val--; //skip down to next vector
	}	
	delete [] u;
	recalculationRequired = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**	This interpolates the spline that has already been defined.
 */
GLboolean ForwardCubicSpline::interpolateAt(float t, float &y)
{	
	int tLow, tHigh;
	float a, b, aCubedMinusA, bCubedMinusB;

	if (recalculationRequired)
		recalculate2ndDerivative();

	tLow = (int)floor(t);
	tHigh = (int)ceil(t);
	if (tHigh >= _n || tLow < 0)
		return GL_FALSE;

	if (tHigh == tLow)
	{
		y = controlPoints[tLow];
		return GL_TRUE;
	}

	a = tHigh - t;
	b = t - tLow;
	aCubedMinusA = (a * a * a - a) / 6.0f;
	bCubedMinusB = (b * b * b - b) / 6.0f;

	y = a * controlPoints[tLow] + b * controlPoints[tHigh] +
			aCubedMinusA * _d2[tLow] + bCubedMinusB * _d2[tHigh];

	return GL_TRUE;
}
