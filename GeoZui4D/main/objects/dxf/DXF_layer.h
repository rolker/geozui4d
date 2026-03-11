// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef DXF_LAYER_H
#define DXF_LAYER_H

#include"engine/SimpleObjectCollection.h"
#include"DXF_thing.h"

///\ingroup DXFGroup
class DXF_layer: public DXF_thing, public SimpleObjectCollection
{
public:
	DXF_layer();
	DERIVE_OBJECT(DXF_LAYER);
	
	void setRGB(float red, float green, float blue);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	int readFrom(FILE * file = NULL);
private:
	float r,g,b;
};


#endif
