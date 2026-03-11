// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
	
