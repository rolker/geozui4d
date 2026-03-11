// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include"DXF_VERTEX.h"
#include<stdlib.h>

#define DXF_VERTEX_3DPOLYLINE 32
#define DXF_VERTEX_3DPOLYGON 64
#define DXF_VERTEX_POLYFACE 128


DXF_VERTEX::DXF_VERTEX()
    :vertexFlags(0),index(4,0)
{
}

int DXF_VERTEX::readFrom(FILE *file)
{
    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 10:
                    point[0]=(float)atof(valueString);
                    break;
                case 20:
                    point[1]=(float)atof(valueString);
                    break;
                case 30:
                    point[2]=(float)atof(valueString);
                    break;
                case 70:
                    vertexFlags = atoi(valueString);
                    break;
                case 71:
                case 72:
                case 73:
                case 74:
                    index[groupCode-71] = atoi(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
    }
    return 0;
}

bool DXF_VERTEX::isPolyfaceMeshVertex()
{
    return vertexFlags & DXF_VERTEX_POLYFACE;
}

bool DXF_VERTEX::is3DPolygonVertex()
{
    return vertexFlags & DXF_VERTEX_3DPOLYGON;
}
	

gz4d::Point<float> const &DXF_VERTEX::getPoint()
{
    return point;
}

int DXF_VERTEX::getIndex(int i)
{
    return index[i-1];
}
