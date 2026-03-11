// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
	
