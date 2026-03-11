// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// glyph.h: base for various objects: cone, cylinder, cube, etc. m 
//
//////////////////////////////////////////////////////////////////////
class Glyph;

#ifndef GLYPH_H
#define GLYPH_H

#include<gz4d/glo.h>
#include "SDV_types.h"

class Glyph
{
public:
	Glyph() {}
	virtual ~Glyph() {}

	enum {Render3D, Render2D_inDepth, Render2D_atScreen};
	virtual void draw(float deg, int renderMode = Render3D) = 0;
        static void CalcNormal(gz4d::Point<double> const &v1, gz4d::Point<double> const &v2, gz4d::Point<double> const &v3, Point3d result, float heightEx = 1.0);
};

#endif

