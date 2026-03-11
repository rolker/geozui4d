// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// invertedCone.h: construct and draw an inverted cone glype
//
/////////////////////////////////////////////////////////////////
class InvertedCone;

#ifndef INVERTED_CONE_H
#define INVERTED_CONE_H

#include "objects/GeoData/Glyph.h"

class InvertedCone : public Glyph
{
public:
	InvertedCone();
	void draw(float deg, int renderMode = Render3D);

private:
	void makeCone();
	std::vector<gz4d::Point<double> > basePoints; // to store the cone vertices
	vNorms * coneNormals; // store the cone normals
};

#endif
