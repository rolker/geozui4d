/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
////////////////////////////////////////////////////////////////
//
// cone.h: construct and draw a cone glype
//
/////////////////////////////////////////////////////////////////
class Cone;

#ifndef CONE_H
#define CONE_H

#include "objects/GeoData/Glyph.h"

class Cone : public Glyph
{
public:
	Cone();
	Cone(float width, float hgtFactor, GLboolean flatShade = GL_TRUE);
	virtual ~Cone(){}
	
	void draw(float deg, int renderMode = Render3D);

private:
	void makeCone();
	std::vector<gz4d::Point<double> > basePoints; // to store the cone vertices
	vNorms * coneNormals; // store the cone normals

	GLboolean flatShading;
	float rad, heightFactor;
};

#endif
