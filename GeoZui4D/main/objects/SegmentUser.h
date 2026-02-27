/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTUSER_H
#define SEGMENTUSER_H

#include "engine/SimpleObjectCollection.h"
#include <gz4d/Color.h>

class Segment;

class SegmentUser : public SimpleObjectCollection
{
public:
    SegmentUser();
protected:
    Segment * newSegment();
    Segment * newSegment(gz4d::Point<double> const &s, gz4d::Point<double> const &e);

    // default 
    std::shared_ptr<gz4d::Color> startColor;
    std::shared_ptr<gz4d::Color> endColor;

    float startWidth, startHeight, endWidth, endHeight;

};

#endif
