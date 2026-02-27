/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////

#include"objects/SegmentUser.h"
#include "objects/Segment.h"

SegmentUser::SegmentUser()
    :startColor(new gz4d::Color),endColor(new gz4d::Color)
{
    startWidth = 1.0;
    startHeight = 1.0;
    endWidth = 1.0;
    endHeight = 1.0;
}

Segment * SegmentUser::newSegment()
{
    Segment * s;

    s = new Segment;
    s->setColor(startColor,endColor);
    s->setScales(&startWidth, &startHeight, &endWidth, &endHeight);

    addObject(s);

    return s;
}

Segment * SegmentUser::newSegment(gz4d::Point<double> const &s, gz4d::Point<double> const &e)
{
    if(IsEmpty())
        SetPosition(s);

    Segment * seg = newSegment();
    gz4d::Point<double> ref = GetPosition();

    seg->setPositions(gz4d::Point<float>(s-ref), gz4d::Point<float>(e-ref));

    return seg;
}


