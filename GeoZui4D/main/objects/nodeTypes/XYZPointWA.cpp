// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "objects/nodeTypes/XYZPointWA.h"
#include "objects/nodeTypes/XYZPointWAHandler.h"
#include <typeinfo>


SDV_DataHandler * XYZPointWA::NewHandler(double x, double y)
{
    return new XYZPointWAHandler(x,y);
}

