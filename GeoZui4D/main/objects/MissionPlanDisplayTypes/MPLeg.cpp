/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"MPLeg.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPLeg, "");

MPLeg::MPLeg(int cid) : MPSegment(cid)
{
	start.SetLabel("start");
	AddObject(&start);

	end.SetLabel("end");
	AddObject(&end);
}

void MPLeg::DrawLowRes(float degradation, Window3D *win)
{
	MPSegment::DrawLowRes(degradation,win);
}

void MPLeg::command(string c)
{
	MPSegment::command(c);
	start.SetPosition(startPos);
	end.SetPosition(endPos);
}
