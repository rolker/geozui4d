/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef SOUNDINGBUFFER_H
#define SOUNDINGBUFFER_H

#include"SDV_types.h"

class SoundingBuffer
{
public:
	SoundingBuffer(int length=1000);
	void Draw(float deg = 0.0);
	void Add(Point3f s);
private:
	Point3f * buffer;
	int head;
	int length;
	int filled;
};

#endif
