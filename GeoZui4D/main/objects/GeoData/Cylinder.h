// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// cylinder.h: construct and draw a cylinder glyph
//
/////////////////////////////////////////////////////////////////
class Cylineder;

#ifndef CYLINDER_H
#define CYLINDER_H

#include "objects/GeoData/Glyph.h"

class Cylinder : public Glyph
{
public:
	Cylinder();
	Cylinder(float rad);
	Cylinder(float rad, float hgtFactor, float orient = 0.0);
	virtual ~Cylinder(){}

	
	void draw(float deg, int renderMode = Render3D);

private:
	void makeCylinder();
	Point3f * Points; // to store the cylinder vertices
	vNorms * Normals; // store the cylinder normals
	float radius;
	float heightFactor, orientation;
};

#endif
