// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "objects/nodeTypes/XYZPoint.h"
#include "objects/nodeTypes/XYZPointHandler.h"
#include <typeinfo>

#include <iostream>

using namespace std;
//namespace UNH_CCOM
//{
    
SDV_DataHandler * XYZPoint::NewHandler(double x, double y)
{
    return new XYZPointHandler;
}

//}
