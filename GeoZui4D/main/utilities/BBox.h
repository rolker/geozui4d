// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef BBOX_H
#define BBOX_H

#include <gz4d/Point.h>

class BBox
{
    public:
        BBox();
        double n,s,e,w,t,b;	// North, south, east, west, top, bottom
        double start, end; // start and end times.
        int merge(BBox bbox); // returns 1 if we got expanded, 0 otherwise
        void expand(gz4d::Point<double> const &point);
        void set(gz4d::Point<double> const &point);
        void set(BBox b);
        void expandTime(double t);
        void expandTime(double t1, double t2);
        int overlapps(BBox bbox);
        bool contains(gz4d::Point<double> const &p);
        int hasVolume();
        void debugPrint();
        void reset();
        
        // test 1 dimention
        int isInX(double x);
        int isInY(double y);
        int isInZ(double z);
};

#endif
