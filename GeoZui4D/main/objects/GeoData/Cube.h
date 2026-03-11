// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// cube.h: construct and draw a cube glype
//
/////////////////////////////////////////////////////////////////
class Cube;

#ifndef CUBE_H
#define CUBE_H

#include "objects/GeoData/Glyph.h"

class Cube : public Glyph
{
public:
	Cube(float width=0.0, float hgt=0.0);
	void draw(float deg, int renderMode = Render3D);
	void init(float width, float height);
	virtual ~Cube(){}

private:
	Point3f Points[8]; // to store the cube vertices
        GLuint displayList;
};

#endif
