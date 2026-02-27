/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef XYZPOINTAVG_H
#define XYZPOINTAVG_H

#include "objects/nodeTypes/SDV_Data.h"


class XYZPointAvg : public SDV_Data
{
public:
    SDV_DataHandler * NewHandler(double x, double y);
};
#endif
