// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef MPGRID_H	
#define MPGRID_H

#include "MPDisplayType.h"

/// \ingroup MissionPlanGroup
/// Series of back and forth lines forming a survey grid.
class MPGrid: public MPDisplayType
{
public:
	MPGrid(int cid=0);
	DERIVE_OBJECT(MPGrid);
	virtual void DrawLowRes(float degradation, Window3D *win);
private:

	float length;
	float width;
	int numLines;
//	MissionPlanNode * startPosition;
//	MissionPlanNode * oppositeCorner;
	
	int adjustLines;
	float maxSeparation;
	int restrictOddEven;
	int restrictToEven;
};

#endif
	
	
	
	
