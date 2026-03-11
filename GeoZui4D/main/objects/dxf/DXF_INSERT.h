// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_INSERT_H
#define DXF_INSERT_H

#include "DXF_entity.h"
class DXF_BLOCK;

///\ingroup DXFGroup
class DXF_INSERT:public DXF_entity
{
public:
	DXF_INSERT();

	int readFrom(FILE *file);
	void Draw(float degradation, Window3D *win);
	void setBlock(DXF_BLOCK * b);

private:
	Point3d reference;
	Point3d scale;
	DXF_BLOCK * block;
	double rotation;
};


#endif
