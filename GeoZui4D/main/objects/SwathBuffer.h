// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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
