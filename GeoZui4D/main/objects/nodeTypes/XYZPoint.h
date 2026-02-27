/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef XYZPOINT_H
#define XYZPOINT_H

#include "objects/nodeTypes/SDV_Data.h"
#include "SDV_types.h"

//namespace UNH_CCOM
//{

class XYZPoint : public SDV_Data
{
public:
    XYZPoint(){};
    SDV_DataHandler * NewHandler(double x, double y);
};

//}

#endif

