#ifndef DISTANCE_H
#define DISTANCE_H

/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
//
// Distance class - created Aug 28, 2001 by rja
// Fast distance approximation of a point from the origin
// A lookup table is created to eliminate sqrt calcs to find  distance.
// The precision of the answer depends on the size of the lookup table.

class Distance
{
public:
    Distance(int s=10000);
    double GetDistance(double x, double y);
private:
	double * table;
	int size;
};

#endif
