/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
