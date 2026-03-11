// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef XYZPOINTAVG_H
#define XYZPOINTAVG_H

#include "objects/nodeTypes/SDV_Data.h"


class XYZPointAvg : public SDV_Data
{
public:
    SDV_DataHandler * NewHandler(double x, double y);
};
#endif
