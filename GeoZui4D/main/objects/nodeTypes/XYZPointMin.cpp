/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/


#include "objects/nodeTypes/XYZPointMin.h"
#include "objects/nodeTypes/XYZPointMinHandler.h"
#include <typeinfo>

SDV_DataHandler * XYZPointMin::NewHandler(double x, double y)
{
    return new XYZPointMinHandler;
}
