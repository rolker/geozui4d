/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
