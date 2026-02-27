/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"MPHome.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPHome, "");

MPHome::MPHome(int cid) : MPPoint(cid)
{
	SET_POINT(colour,0.6,0.6,0.9);
	pointSize = 6.0;
	sphereSize = 0.075;
}

void MPHome::DrawLowRes(float degradation, Window3D *win)
{
	MPPoint::DrawLowRes(degradation,win);
}

