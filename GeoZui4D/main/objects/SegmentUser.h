// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
