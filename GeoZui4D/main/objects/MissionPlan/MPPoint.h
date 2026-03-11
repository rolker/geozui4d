// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef MPPOINT_H	
#define MPPOINT_H

#include "MPItemBase.h"

/// \ingroup MissionPlanGroup
/// Generic point
class MPPoint: public MPItemBase
{
public:
	MPPoint();
    MPPoint(int id, gz4d::xml::Node cmd, int pickType = PICK_CLICK1 | PICK_PRESS3);
	DERIVE_OBJECT(MPPoint);

	virtual void DrawLowRes(float degradation, Window3D *win);
    virtual void command(gz4d::xml::Node c, std::string const &cmd);
private:
};

#endif
