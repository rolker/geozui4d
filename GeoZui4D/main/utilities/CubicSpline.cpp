/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// CubicSpline.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class handles cubic-spline resampling of arrays of vectors.  It
 *	assumes evenly spaced values in the t domain to increase its efficiency.
 *
 *	Converted by mdp 2/13/2002 from Colin's CubicSp class, which in
 *	turn was an adaptation of Numerical Recipes in C's implementation.
 */
/*---------------------------------------------------------------------------*/
#include "utilities/CubicSpline.h"
#include "widgets/Sv_Standards.h"
#include <math.h>
    
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
CubicSpline::CubicSpline(std::vector<gz4d::Point<double> > const &points)
{
    setControlPoints(points);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void CubicSpline::setControlPoints(std::vector<gz4d::Point<double> > const &points)
{
    controlPoints = points;

    if (points.empty())
        return;

    delta.resize(points.size());
    _d2.resize(points.size());

    //initialize deltas.
    std::vector<gz4d::Point<double> >::iterator currentDelta = delta.begin();
    *currentDelta = gz4d::Point<double>(0.0,0.0,0.0);
    ++currentDelta;
    std::vector<gz4d::Point<double> >::iterator previousPoint = controlPoints.begin();
    std::vector<gz4d::Point<double> >::iterator currentPoint = controlPoints.begin();
    ++currentPoint;
    while(currentPoint != controlPoints.end())
            *currentDelta++ = *currentPoint++ - *previousPoint++;

    recalculationRequired = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void CubicSpline::pointUpdated(unsigned int index)
{
    std::vector<gz4d::Point<double> >::iterator currentDelta;
    std::vector<gz4d::Point<double> >::iterator currentPoint, previousPoint;

	if (index > 0)
	{
		currentDelta = delta.begin()+index;
		previousPoint = controlPoints.begin()+index - 1;
		currentPoint = previousPoint;
                ++currentPoint;
		*currentDelta++ = *currentPoint++ - *previousPoint++;
	}
	else
	{
		currentDelta = delta.begin()+1;
                previousPoint = controlPoints.begin()+index;
                currentPoint = previousPoint;
                ++currentPoint;
	}
	if (index < delta.size())
	{
                *currentDelta++ = *currentPoint++ - *previousPoint++;
	}
	recalculationRequired = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void CubicSpline::recalculate2ndDerivative()
{
	int k, end;
	double p;
        std::vector< gz4d::Point<double> > u;
        std::vector<gz4d::Point<double> >::iterator currentDelta, previousDelta, currentU, previousU,d2Val,currentD2, nextD2;
	
	u.resize(controlPoints.size());
	
	previousU = currentU = u.begin();
        currentDelta = delta.begin()+1;
	d2Val = _d2.begin();

	(*d2Val)[0] = -0.5f; //can do this since identical across dimensions here
        *currentU = (*currentDelta)*3.0;
        ++currentU;
        ++currentDelta;
	
	//work forwards
	previousDelta = delta.begin()+1;
	end = controlPoints.size() - 1;
	for (k = 1; k < end; k++) 
	{
		p = (*d2Val)[0] + 4.0f;
		++d2Val; //skip to next vector
		(*d2Val)[0] = (-1.0f) / p;
                *currentU++ =((*currentDelta++ - *previousDelta++)*6.0f - (*previousU++)) / p;
	}
	
	p = (*d2Val)[0] + 2.0f;
	currentD2 = d2Val +1; //skip to next vector

        *currentD2++ = ((*previousDelta++) *6.0 - (*previousU++)) / p;
	
	//now work backwards
	currentD2 = _d2.end()-2;
        nextD2 = _d2.end()-1;
        d2Val = currentD2;
	currentU = u.end()-2;
	for (k= controlPoints.size()-2; k >= 0; k--)
	{
		*currentD2-- = *d2Val * (*nextD2--) + *currentU--; //Z
		*currentD2-- = *d2Val * (*nextD2--) + *currentU--; //Y
		*currentD2-- = *d2Val * (*nextD2--) + *currentU--; //X
		--d2Val; //skip down to next vector
	}	
	recalculationRequired = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**	output resampled values into resamplesPoints (of length m)
 *	typically m >> _n
 */
GLboolean CubicSpline::resample(std::vector<gz4d::Point<double> > &resampledPoints)
{
    double step;
    GLboolean retval = GL_TRUE;
    
    step = (controlPoints.size() - 1) / (float)resampledPoints.size();
    
    for (unsigned int i = 0; i < resampledPoints.size(); i++)
		retval &= interpolateAt(i * step, resampledPoints[i]);

	return retval;
}

/*---------------------------------------------------------------------------*/
/**	This interpolates the spline that has already been defined.
 */
GLboolean CubicSpline::interpolateAt(double t, gz4d::Point<double> &point)
{	
	unsigned int tLow, tHigh, i=0;
	double a, b, aCubedMinusA, bCubedMinusB;

	if (recalculationRequired)
		recalculate2ndDerivative();

        if (!point.IsValid())
		return GL_FALSE;

	tLow = (int)floor(t);
	tHigh = (int)ceil(t);
	if (tHigh >= controlPoints.size() || tLow < 0)
		return GL_FALSE;

	if (tHigh == tLow)
	{
		point = controlPoints[tLow];
		return GL_TRUE;
	}

	a = tHigh - t;
	b = t - tLow;
	aCubedMinusA = (a * a * a - a) / 6.0f;
	bCubedMinusB = (b * b * b - b) / 6.0f;

        point = controlPoints[tLow]*a + controlPoints[tHigh]*b + _d2[tLow]*aCubedMinusA  + _d2[tHigh][i]* bCubedMinusB;

	return GL_TRUE;
}
