/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include <math.h>
#include "objects/GeoData/SphereGlyph.h"
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "engine/ToglWindow.h"

#define RADIUS	1.5

//2D representation
static unsigned char outline[32] = {0x07, 0xE0,
								  0x18, 0x18,
								  0x20, 0x04,
								  0x40, 0x02,
								  0x40, 0x02,
								  0x80, 0x01,
								  0x80, 0x01,
								  0x80, 0x01,
								  0x80, 0x01,
								  0x80, 0x01,
								  0x80, 0x01,
								  0x40, 0x02,
								  0x40, 0x02,
								  0x20, 0x04,
								  0x18, 0x18,
								  0x07, 0xE0};

static unsigned char filling[32] = {0x00, 0x00,
								  0x07, 0xE0,
								  0x1F, 0xF8,
								  0x3F, 0xFC,
								  0x3F, 0xFC,
								  0x7F, 0xFE,
								  0x7F, 0xFE,
								  0x7F, 0xFE,
								  0x7F, 0xFE,
								  0x7F, 0xFE,
								  0x7F, 0xFE,
								  0x3F, 0xFC,
								  0x3F, 0xFC,
								  0x1F, 0xF8,
								  0x07, 0xE0,
								  0x00, 0x00};

/*--------------------------------------------------------------------------*/
SphereGlyph::SphereGlyph()
{
	radius = RADIUS;
}

/*--------------------------------------------------------------------------*/
SphereGlyph::SphereGlyph(float rad)
{
	radius = rad;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void SphereGlyph::draw(float deg, int renderMode)
{
	glShadeModel(GL_SMOOTH);
	
	if (renderMode != Render3D) //using2D
	{
		float x = 0.0, y = 0.0, z = 0.0;

		if (renderMode == Render2D_atScreen)
		{
			if (!ToglWindow::beginRaster2D(x, y, z)) //not at a valid screen location
				return;
		}
		else
			glRasterPos3f(x, y, z);

		glBitmap(16, 16, 8, 8, 0, 0, filling);
		glPushAttrib(GL_CURRENT_BIT);
		glColor4f(0.0, 0.0, 0.0, 1.0);

		glRasterPos3f(x, y, z);
		glBitmap(16, 16, 8, 8, 0, 0, outline);
		glPopAttrib();

		if (renderMode == Render2D_atScreen)
			ToglWindow::endRaster2D();
		return;
	}

	if (deg > 0.1)
		glutSolidSphere(radius,5,6);
	else
#ifdef EXPERIMENT_GLYPHS
		glutSolidSphere(RADIUS,10,9);
#else
		glutSolidSphere(radius,20,18);
#endif
}


