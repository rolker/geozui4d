/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef SWATHBUFFER_H
#define SWATHBUFFER_H

#include"SDV_types.h"
#include <gz4d/Point.h>

typedef std::vector<gz4d::Point<float> > SwathSoundings;

struct SwathType
{
    SwathSoundings soundings;
    short soundingCount;
    float east;
    float north;
    float roll;
    float pitch;
    float heading;
    float heave;    
};


class SwathBuffer
{
public:
    SwathBuffer(gz4d::Point<double> const &objoffset, int length=100);
	void Draw(float deg = 0.0);
	void Add(SwathType s);
private:
	SwathType * buffer;
	int head;
	int length;
	int filled;
	gz4d::Point<float> offset;
};

#endif
