/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
