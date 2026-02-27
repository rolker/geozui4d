/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
