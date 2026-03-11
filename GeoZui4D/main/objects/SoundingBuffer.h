// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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
