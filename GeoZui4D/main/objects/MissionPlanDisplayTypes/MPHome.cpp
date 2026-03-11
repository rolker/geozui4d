// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"MPHome.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPHome, "");

MPHome::MPHome(int cid) : MPPoint(cid)
{
	SET_POINT(colour,0.6,0.6,0.9);
	pointSize = 6.0;
	sphereSize = 0.075;
}

void MPHome::DrawLowRes(float degradation, Window3D *win)
{
	MPPoint::DrawLowRes(degradation,win);
}

