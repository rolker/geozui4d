/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// ForwardCubicSpline.h
/*---------------------------------------------------------------------------*/

class ForwardCubicSpline;

#ifndef FORWARDCUBICSPLINE_H
#define FORWARDCUBICSPLINE_H

#include "SDV_types.h"

/** 
 *	This class handles cubic-spline resampling of arrays of floats.  It
 *	assumes evenly spaced values in the input (t) domain to increase its
 *	efficiency (y varies).
 *  \note from CubicSpline and InverseCubicSpline, which in turn were adapted 
 *  from Colin's CubicSp class, which in turn was an adaptation of 
 *  Numerical Recipes in C's implementation.
 *	\author (Converted by) Matt Plumllee (mdp)
 *  \date 4/10/2003 
 */
/*---------------------------------------------------------------------------*/
class ForwardCubicSpline
{
	public:
		ForwardCubicSpline(const float *points, int n);
		~ForwardCubicSpline();

		//---Mutators---
		void setControlPoints(const float *points, int n);
		void pointUpdated(int index);

		//---Accessors---
		const float *getControlPoints() {return controlPoints;}
		int getLength();

		//---Operations---
		GLboolean interpolateAt(float t, float &y);
	
	private:
		void recalculate2ndDerivative();

		const float *controlPoints;
		float *delta; // this stores the first-order deltas
		float *_d2; // this stores the second derivative
		int _n;  // number of control points

		GLboolean recalculationRequired;
};

#endif
