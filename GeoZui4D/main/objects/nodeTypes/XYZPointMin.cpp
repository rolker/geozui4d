// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0


#include "objects/nodeTypes/XYZPointMin.h"
#include "objects/nodeTypes/XYZPointMinHandler.h"
#include <typeinfo>

SDV_DataHandler * XYZPointMin::NewHandler(double x, double y)
{
    return new XYZPointMinHandler;
}
