#ifndef DISTANCE_H
#define DISTANCE_H

// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
