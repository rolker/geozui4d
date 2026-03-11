// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"MPGoto.h"

REGISTER_OBJECT(MPGoto, "");

MPGoto::MPGoto(int cid) : MPPoint(cid)
{
	SET_POINT(colour,0.6,0.6,0.9);
	pointSize = 6.0;
	sphereSize = 0.075;
}

void MPGoto::DrawLowRes(float degradation, Window3D *win)
{
	MPPoint::DrawLowRes(degradation,win);
}

