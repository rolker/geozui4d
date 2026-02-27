/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "DoublePyramid.h"
#include "engine/ToglWindow.h"
#define SIZE	1.3f

//2D representation
static unsigned char outline[32] = {0x00, 0x00,
								  0x00, 0x80,
								  0x01, 0x40,
								  0x02, 0x20,
								  0x04, 0x10,
								  0x08, 0x08,
								  0x10, 0x04,
								  0x20, 0x02,
								  0x40, 0x01,
								  0x20, 0x02,
								  0x10, 0x04,
								  0x08, 0x08,
								  0x04, 0x10,
								  0x02, 0x20,
								  0x01, 0x40,
								  0x00, 0x80};

static unsigned char filling[32] = {0x00, 0x00,
								  0x00, 0x00,
								  0x00, 0x80,
								  0x01, 0xC0,
								  0x03, 0xE0,
								  0x07, 0xF0,
								  0x0F, 0xF8,
								  0x1F, 0xFC,
								  0x3F, 0xFE,
								  0x1F, 0xFC,
								  0x0F, 0xF8,
								  0x07, 0xF0,
								  0x03, 0xE0,
								  0x01, 0xC0,
								  0x00, 0x80,
								  0x00, 0x00};

/*--------------------------------------------------------------------------*/
DoublePyramid::DoublePyramid()
{
	// top 
	Points[0][0] = 0;
	Points[0][1] = 0;
	Points[0][2] = 1.3*SIZE;

	Points[1][0] = -SIZE;
	Points[1][1] = -SIZE;
	Points[1][2] = 0;

	Points[2][0] = -SIZE;
	Points[2][1] = SIZE;
	Points[2][2] = 0;

	Points[3][0] = SIZE;
	Points[3][1] = SIZE;
	Points[3][2] = 0;

	Points[4][0] = SIZE;
	Points[4][1] = -SIZE;
	Points[4][2] = 0;

	Points[5][0] = 0;
	Points[5][1] = 0;
	Points[5][2] = -1.3*SIZE;


}

/*--------------------------------------------------------------------------*/
void DoublePyramid::draw(float deg, int renderMode)
{
	glShadeModel(GL_FLAT);

#ifdef EXPERIMENT_GLYPHS
	glRotatef(45.0, 0.0, 0.0, 1.0);
#endif

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

	//draw the top part
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-0.6f, 0.0, 0.8f);
	glVertex3fv(Points[0]); 
	glVertex3fv(Points[1]); 
	glVertex3fv(Points[2]); 
		glNormal3f(0.0, 0.6f, 0.8f);
	glVertex3fv(Points[3]); 
	glNormal3f(0.6f, 0.0, 0.8f);
	glVertex3fv(Points[4]);
	glNormal3f(0.0, -0.6f, 0.8f);
	glVertex3fv(Points[1]);
	glEnd();

	//draw the bottom part
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-0.6f, 0.0, -0.8f);
	glVertex3fv(Points[5]); 
	glVertex3fv(Points[1]); 
	glVertex3fv(Points[2]);
	glNormal3f(0.0, 0.6f, -0.8f);
	glVertex3fv(Points[3]); 
	glNormal3f(0.6f, 0.0, -0.8f);
	glVertex3fv(Points[4]); 
	glNormal3f(0.0, -0.6f, -0.8f);
	glVertex3fv(Points[1]);
	glEnd();
}









