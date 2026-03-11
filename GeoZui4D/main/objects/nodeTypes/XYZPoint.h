// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef XYZPOINT_H
#define XYZPOINT_H

#include "objects/nodeTypes/SDV_Data.h"
#include "SDV_types.h"

//namespace UNH_CCOM
//{

class XYZPoint : public SDV_Data
{
public:
    XYZPoint(){};
    SDV_DataHandler * NewHandler(double x, double y);
};

//}

#endif

