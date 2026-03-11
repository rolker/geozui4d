// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef SDV_DATA_H
#define SDV_DATA_H
#include "SDV_types.h"

/**
SDV_Data is used as an abstract data type for the SDV_Quadtree structure. 
Since the SDV_Quadtree structure is designed to contain datapoints, such
as soundings, at the leaf nodes, which have a preset resolution, it is
possible to have more than one datapoint assigned to a leaf node. This 
class is responsible for managing multiple datapoints assigned to a node,
which can also be seen as multiple soundings in a bin.
*/

class SDV_DataHandler;

class SDV_Data
{
public:
    SDV_Data();
    virtual ~SDV_Data(){};
    virtual SDV_DataHandler * NewHandler(double x, double y){return NULL;};
    Point3f sounding;
    float alternateData;
protected:
};

#endif 
