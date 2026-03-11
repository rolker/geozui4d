// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_ARC_H
#define DXF_ARC_H

#include "DXF_entity.h"

///\ingroup DXFGroup
class DXF_ARC:public DXF_entity
{
public:
	DXF_ARC();

	int readFrom(FILE *file);
	void Draw(float degradation, Window3D *win);

private:
	Point3f center;
	float radius;
	float startAngle;
	float endAngle;
};

#endif
