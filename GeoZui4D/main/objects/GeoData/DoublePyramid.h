// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// doublePyramid.h: construct and draw a double pyramid glype
//
/////////////////////////////////////////////////////////////////
class DoublePyramid;

#ifndef DOUBLE_PYRAMID_H
#define DOUBLE_PYRAMID_H

#include "objects/GeoData/Glyph.h"

class DoublePyramid : public Glyph
{
public:
	DoublePyramid();
	void draw(float deg, int renderMode = Render3D);

private:
	Point3f Points[6]; // to store the cube vertices
};

#endif
