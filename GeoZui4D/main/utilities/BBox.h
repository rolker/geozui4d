/* Copyright (c) 2002 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

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
