// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef XYZPOINTWA_H
#define XYZPOINTWA_H

#include "objects/nodeTypes/SDV_Data.h"
#include "SDV_types.h"


class XYZPointWA : public SDV_Data
{
public:
    XYZPointWA(){radius = 0.0;};
    SDV_DataHandler * NewHandler(double x, double y);
    float radius;
};
#endif
