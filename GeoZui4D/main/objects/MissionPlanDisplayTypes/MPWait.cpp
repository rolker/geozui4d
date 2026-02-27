/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"MPWait.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPWait, "");

MPWait::MPWait(int cid) : MPDisplayType(cid)
{
}

void MPWait::DrawLowRes(float degradation, Window3D *win)
{
}

