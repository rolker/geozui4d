// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"MPWait.h"
#include"engine/Window3D.h"

REGISTER_OBJECT(MPWait, "");

MPWait::MPWait(int cid) : MPDisplayType(cid)
{
}

void MPWait::DrawLowRes(float degradation, Window3D *win)
{
}

