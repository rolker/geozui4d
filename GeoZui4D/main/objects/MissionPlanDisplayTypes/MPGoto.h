/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPGOTO_H	
#define MPGOTO_H

#include "MPPoint.h"

/// \ingroup MissionPlanGroup
/// Goto a position without needing to follow a specific path.
class MPGoto: public MPPoint
{
public:
	MPGoto(int cid=0);
	DERIVE_OBJECT(MPGoto);
	
	virtual void DrawLowRes(float degradation, Window3D *win);
private:
};

#endif
	
