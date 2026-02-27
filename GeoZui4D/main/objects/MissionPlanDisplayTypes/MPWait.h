/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPWAIT_H	
#define MPWAIT_H

#include "MPDisplayType.h"


/// \ingroup MissionPlanGroup
/// Just wait.
class MPWait: public MPDisplayType
{
public:
	MPWait(int cid=0);
	DERIVE_OBJECT(MPWait);
	virtual void DrawLowRes(float degradation, Window3D *win);
private:
};

#endif
	
