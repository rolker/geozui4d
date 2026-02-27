/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include <math.h>
#include "objects/GeoData/Glyph.h"

/*--------------------------------------------------------------------------*/
void Glyph::CalcNormal(gz4d::Point<double> const &v1, gz4d::Point<double> const &v2, gz4d::Point<double> const &v3, Point3d result, float heightEx)
{
	// do the cross product defined by the vectors
	// v3 - v2, v3 - v1

	double dx1, dy1, dz1, dx2, dy2, dz2;
	double length;

	dx1 = v3[0] - v1[0];
	dy1 = v3[1] - v1[1];
	dz1 = (v3[2] - v1[2]) * heightEx;
	dx2 = v2[0] - v1[0];
	dy2 = v2[1] - v1[1];
	dz2 = (v2[2] - v1[2]) * heightEx;

	// calculate the cross product

	result[0] = dy1*dz2 - dz1*dy2;
	result[1] = dz1*dx2 - dx1*dz2;
	result[2] = dx1*dy2 - dy1*dx2;

	// and normalize to a unit vector
	length = sqrt(result[0]*result[0] 
		+ result[1]*result[1] + result[2]*result[2]);

	result[0] /= length;
	result[1] /= length;
	result[2] /= length;

}
