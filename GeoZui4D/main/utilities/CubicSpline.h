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

class CubicSpline;

#ifndef CUBICSPLINE_H
#define CUBICSPLINE_H

#include "SDV_types.h"
/** 
 *	This class handles cubic-spline resampling of arrays of Point3f's.  It
 *	assumes evenly spaced values in the t domain to increase its efficiency.
 *	\note from Colin's CubicSp class, which in turn was an adaptation of 
 *  Numerical Recipes in C's implementation.
 *	\author (Converted by) Matt Plumlee (mdp)
 *  \date 2/13/2002 
 */
/*---------------------------------------------------------------------------*/
class CubicSpline
{
    public:
        CubicSpline(std::vector<gz4d::Point<double> > const &points);

        //---Mutators---
        void setControlPoints(std::vector<gz4d::Point<double> > const &points);
        void pointUpdated(unsigned int index);

        //---Accessors---
        std::vector<gz4d::Point<double> > const &getControlPoints() {return controlPoints;}
        int getLength();

        //---Operations---
        GLboolean resample(std::vector<gz4d::Point<double> > &resampledPoints);
        GLboolean interpolateAt(double t, gz4d::Point<double> &point);
    
    private:
        void recalculate2ndDerivative();

        std::vector<gz4d::Point<double> > controlPoints;
        std::vector<gz4d::Point<double> > delta; // this stores the first-order deltas
        std::vector<gz4d::Point<double> > _d2; // this stores the second derivative

        GLboolean recalculationRequired;
};

#endif
