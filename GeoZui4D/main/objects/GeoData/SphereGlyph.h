// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// sphereGlyph.h: construct and draw a sphere glype
//
/////////////////////////////////////////////////////////////////
class SphereGlyph;

#ifndef SPHERE_GLYPH_H
#define SPHERE_GLYPH_H

#include "objects/GeoData/Glyph.h"

class SphereGlyph : public Glyph
{
public:
	SphereGlyph();
	SphereGlyph(float rad);
	virtual ~SphereGlyph(){}
	void draw(float deg, int renderMode = Render3D);
private:
	float radius;
	
};

#endif
