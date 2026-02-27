/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/


#include "objects/nodeTypes/XYZPointAvg.h"
#include "objects/nodeTypes/XYZPointAvgHandler.h"
#include <typeinfo>

SDV_DataHandler * XYZPointAvg::NewHandler(double x, double y)
{
    return new XYZPointAvgHandler;
}
