// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
	
