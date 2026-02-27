/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPHOME_H	
#define MPHOME_H

#include "MPPoint.h"

/// \ingroup MissionPlanGroup
/// Home position
class MPHome: public MPPoint
{
public:
	MPHome(int cid=0);
	DERIVE_OBJECT(MPHome);
	
	virtual void DrawLowRes(float degradation, Window3D *win);

private:
};

#endif
	
