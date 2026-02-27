/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "objects/nodeTypes/XYZPointWA.h"
#include "objects/nodeTypes/XYZPointWAHandler.h"
#include <typeinfo>


SDV_DataHandler * XYZPointWA::NewHandler(double x, double y)
{
    return new XYZPointWAHandler(x,y);
}

