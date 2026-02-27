/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include <gz4d/gz4d.h>
#include "objects/nodeTypes/SDV_Data.h"
#include <gz4d/util/Utilities.h>

SDV_Data::SDV_Data()
{
    sounding[0] = sounding[1] = sounding[2] = 0.0;
    alternateData = gz4d::Nan<float>();
}
