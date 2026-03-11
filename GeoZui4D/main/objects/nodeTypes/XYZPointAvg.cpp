// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0


#include "objects/nodeTypes/XYZPointAvg.h"
#include "objects/nodeTypes/XYZPointAvgHandler.h"
#include <typeinfo>

SDV_DataHandler * XYZPointAvg::NewHandler(double x, double y)
{
    return new XYZPointAvgHandler;
}
