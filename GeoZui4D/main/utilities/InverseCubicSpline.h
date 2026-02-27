/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// InverseCubicSpline.h
/*---------------------------------------------------------------------------*/

class InverseCubicSpline;

#ifndef INVERSECUBICSPLINE_H
#define INVERSECUBICSPLINE_H

#include "SDV_types.h"
/** 
 *	This class handles inverse cubic-spline resampling of arrays of floats.  It
 *	assumes evenly spaced values in the output (y) domain to increase its
 *	efficiency (t varies).  These y-values correspond to indexes into regular
 *	cubic splines.
 *	\note from CubicSpline and Colin's CubicSp class, which
 *	in turn was an adaptation of Numerical Recipes in C's implementation.
 *	\author (Converted by) Matt Plumlee (mdp)
 *  \date 3/1/2003 
 */
/*---------------------------------------------------------------------------*/
class InverseCubicSpline
{
	public:
		InverseCubicSpline(const float *points, int n);
		~InverseCubicSpline();

		int previous_tHighIndex, previous_tLowIndex;

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
