// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"MPLeg.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPLeg, "");

MPLeg::MPLeg(int cid) : MPSegment(cid)
{
	start.SetLabel("start");
	AddObject(&start);

	end.SetLabel("end");
	AddObject(&end);
}

void MPLeg::DrawLowRes(float degradation, Window3D *win)
{
	MPSegment::DrawLowRes(degradation,win);
}

void MPLeg::command(string c)
{
	MPSegment::command(c);
	start.SetPosition(startPos);
	end.SetPosition(endPos);
}
