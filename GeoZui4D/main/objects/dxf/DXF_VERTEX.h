// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_VERTEX_H
#define DXF_VERTEX_H

#include<stdio.h>
#include <gz4d/Point.h>
#include <vector>

#include "DXF_thing.h"

///\ingroup DXFGroup
class DXF_VERTEX:public DXF_thing
{
public:
    DXF_VERTEX();

    int readFrom(FILE *file = NULL);
    gz4d::Point<float> const &getPoint();
    bool isPolyfaceMeshVertex();
    bool is3DPolygonVertex();
    int getIndex(int i);
private:
    gz4d::Point<float> point;

    int vertexFlags;
    std::vector<int> index;
};


#endif
