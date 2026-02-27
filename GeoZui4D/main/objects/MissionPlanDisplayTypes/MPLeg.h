/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef MPLEG_H	
#define MPLEG_H

#include "MPSegment.h"
#include "MPPoint.h"

/// \ingroup MissionPlanGroup
/// A leg from point A to point B
class MPLeg: public MPSegment
{
public:
	MPLeg(int cid=0);
	DERIVE_OBJECT(MPLeg);
	virtual void DrawLowRes(float degradation, Window3D *win);
	virtual void command(string c);
private:
	MPPoint start;
	MPPoint end;
};

#endif
	
