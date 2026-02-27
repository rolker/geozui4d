/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
