// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_BLOCK_H
#define DXF_BLOCK_H

#include"engine/SimpleObjectCollection.h"
#include"DXF_thing.h"

class DXF;

///\ingroup DXFGroup
class DXF_BLOCK:public DXF_thing, public SimpleObjectCollection
{
public:
	DXF_BLOCK();
	DERIVE_OBJECT(DXF_BLOCK);

	int readFrom(FILE * file = NULL);
	void setDXF(DXF * d){dxfManager = d;};
private:
	Point3f base;
	int flags;
	DXF * dxfManager;
};

#endif
