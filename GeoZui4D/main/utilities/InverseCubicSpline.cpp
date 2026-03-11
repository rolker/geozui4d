// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// InverseCubicSpline.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class handles inverse cubic-spline resampling of arrays of floats.  It
 *	assumes evenly spaced values in the output (y) domain to increase its
 *	efficiency (t varies).  These y-values correspond to indexes into regular
 *	cubic splines.
 *
 *	Converted by mdp 3/1/2003 from CubicSpline and Colin's CubicSp class, which
 *	in turn was an adaptation of Numerical Recipes in C's implementation.
 */
/*---------------------------------------------------------------------------*/
#include "utilities/InverseCubicSpline.h"
#include "widgets/Sv_Standards.h"
#include <math.h>
    
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
InverseCubicSpline::InverseCubicSpline(const float *points, int n)
{
	delta = NULL;
	_d2 = NULL;
	_n = -1;
	setControlPoints(points, n);
}

/*---------------------------------------------------------------------------*/
InverseCubicSpline::~InverseCubicSpline()
{
	delete [] delta;
	delete [] _d2;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void InverseCubicSpline::setControlPoints(const float *points, int n)
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
	previous_tHighIndex = 1;
	previous_tLowIndex = 0;
}

/*---------------------------------------------------------------------------*/
void InverseCubicSpline::pointUpdated(int index)
{
	if (index > 0)
		delta[index] = controlPoints[index] - controlPoints[index - 1];
	if (index < _n)
		delta[index + 1] = controlPoints[index + 1] - controlPoints[index];
	recalculationRequired = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void InverseCubicSpline::recalculate2ndDerivative()
{
	int k, end;
	float p, sig;
	float *u;
	float *currentDelta, *previousDelta, *currentU, *previousU;
	float *nextD2, *d2Val;
	
	if (!_d2)
		return;

	u = new float[_n];
	
	previousU = currentU = u;
	currentDelta = &delta[1];
	d2Val = _d2;

	*d2Val = -0.5f;
	*currentU++ = 3.0f / (*currentDelta * *currentDelta);
	currentDelta++;
	
	//work forwards
	previousDelta = &delta[1];
	end = _n - 1;
	for (k = 1; k < end; k++) 
	{
		sig = *previousDelta / (*previousDelta + *currentDelta);
		p = sig * *d2Val++ + 2.0f;
		*d2Val= (sig - 1.0f) / p;
		*currentU = ((1.0f / *currentDelta) - (1.0f / *previousDelta));
		*currentU = (6.0f * *currentU / (*currentDelta++ + *previousDelta++) - sig * *previousU++) / p;
		currentU++;
	}
	
	p = *d2Val++ + 2.0f;

	*d2Val = ((6.0f / (*previousDelta * *previousDelta)) - *previousU) / p;
	
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
GLboolean InverseCubicSpline::interpolateAt(float t, float &y)
{	
	int tLow, tHigh, splitDiff;
	float a, b, aCubedMinusA, bCubedMinusB, tdiff;

	if (recalculationRequired)
		recalculate2ndDerivative();

	if (!controlPoints) //no control points? Fail
		return GL_FALSE;

	if (t < controlPoints[0] || t > controlPoints[_n - 1]) //this check makes lines below safe to execute (*)
		return GL_FALSE;

	if (t < controlPoints[previous_tLowIndex])
	{
		tHigh = previous_tLowIndex;
		if (t >= controlPoints[tHigh - 1]) //(*)tHigh must be >= 1 to even get here, so we are safe
			tLow = tHigh - 1;
		else
			tLow = 0;
	}
	else if (t >= controlPoints[previous_tHighIndex])
	{
		tLow = previous_tHighIndex;
		if (t < controlPoints[tLow + 1]) //(*)again, we're safe... must be < and not <= because we want == to happen only on tLow
			tHigh = tLow + 1;
		else
			tHigh = _n - 1;
	}
	else
	{
		tHigh = previous_tHighIndex;
		tLow = previous_tLowIndex;
	}

	while (tHigh - tLow > 1)
	{
		splitDiff = (tHigh + tLow) >> 1;
		if (t < controlPoints[splitDiff])
			tHigh = splitDiff;
		else
			tLow = splitDiff;
	}

	previous_tHighIndex = tHigh;
	previous_tLowIndex = tLow;

	tdiff = controlPoints[tHigh] - controlPoints[tLow];
	if (tdiff == 0.0f)
	{
		y = (tHigh + tLow)/2.0f; //just return the difference split, since any value in the range is quasi-valid
		return GL_TRUE;
	}

	a = (controlPoints[tHigh] - t) / tdiff;
	b = (t - controlPoints[tLow]) / tdiff;
	aCubedMinusA = (a * a * a - a);
	bCubedMinusB = (b * b * b - b);

	y = a * tLow + b * tHigh + (aCubedMinusA * _d2[tLow] + bCubedMinusB * _d2[tHigh])/6.0f;

	return GL_TRUE;
}
