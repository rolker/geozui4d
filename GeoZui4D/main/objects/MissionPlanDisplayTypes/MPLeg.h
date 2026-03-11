// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
	
