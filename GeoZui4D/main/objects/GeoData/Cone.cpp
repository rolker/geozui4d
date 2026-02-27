/* Copyright (c) 2001 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include <math.h>
#include "objects/GeoData/Cone.h"
#include "engine/ToglWindow.h"
#include <gz4d/glo/Vertex.h>

#ifdef EXPERIMENT_GLYPHS
#define CONE_SEGS	15
#define LO_RES_STEP 6
#define CONE_RADIUS	1.5
#else
#define CONE_SEGS	30
#define LO_RES_STEP 6
#define CONE_RADIUS	2.0
#endif
#define PI	float(3.141592)

//2D representation
static unsigned char outline[32] = {0x00, 0x00,
								  0x7F, 0xFF,
								  0x40, 0x01,
								  0x20, 0x02,
								  0x20, 0x02,
								  0x10, 0x04,
								  0x10, 0x04,
								  0x08, 0x08,
								  0x08, 0x08,
								  0x04, 0x10,
								  0x04, 0x10,
								  0x02, 0x20,
								  0x02, 0x20,
								  0x01, 0x40,
								  0x01, 0x40,
								  0x00, 0x80};

static unsigned char filling[32] = {0x00, 0x00,
								  0x00, 0x00,
								  0x3F, 0xFE,
								  0x1F, 0xFC,
								  0x1F, 0xFC,
								  0x0F, 0xF8,
								  0x0F, 0xF8,
								  0x07, 0xF0,
								  0x07, 0xF0,
								  0x03, 0xE0,
								  0x03, 0xE0,
								  0x01, 0xC0,
								  0x01, 0xC0,
								  0x00, 0x80,
								  0x00, 0x80,
								  0x00, 0x00};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Cone::Cone()
{
	coneNormals = new vNorms[CONE_SEGS];
	basePoints.resize(CONE_SEGS+2);

	rad = float(CONE_RADIUS);
	heightFactor = 0.5f;
	flatShading = GL_TRUE;
	makeCone();
}

/*---------------------------------------------------------------------------*/
Cone::Cone(float width, float hgtFactor, GLboolean flatShade)
{
	coneNormals = new vNorms[CONE_SEGS];
        basePoints.resize(CONE_SEGS+2);

	rad = width;
	heightFactor = hgtFactor;
	flatShading = flatShade;
	makeCone();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Cone::draw(float deg, int renderMode)
{
	int i,step;

	step = 1;
	if (deg > 0.1) step = LO_RES_STEP;

	if (flatShading)
		glShadeModel(GL_FLAT);
	else
		glShadeModel(GL_SMOOTH);
#ifdef EXPERIMENT_GLYPHS
	glRotatef(-70.0, 1.0, 0.0, 0);
#endif

	if (renderMode == Render3D)
	{
		//do pairs of points
		glBegin(GL_TRIANGLE_FAN);
		gz4d::glo::Vertex(basePoints[CONE_SEGS]);
		for (i = 0; i < CONE_SEGS; i+=step)
		{
			//glNormal3s(coneNormals[i].nx, coneNormals[i].ny, coneNormals[i].nz);
			glNormal3sv(coneNormals[i]);
                        gz4d::glo::Vertex(basePoints[i]);
		}
		//glNormal3s(coneNormals[0].nx, coneNormals[0].ny, coneNormals[0].nz);
		glNormal3sv(coneNormals[0]);
                gz4d::glo::Vertex(basePoints[0]);
		glEnd();
	}
	else //using2D
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
	}
}

/*---------------------------------------------------------------------------*/
void Cone::makeCone()
{
	Point3d normal;
	int i;
	float x,y,z,deg;

	basePoints[CONE_SEGS][0] = 0.0;
	basePoints[CONE_SEGS][1] = 0.0;
#ifdef EXPERIMENT_GLYPHS
	basePoints[CONE_SEGS][2] = rad;
#else
	basePoints[CONE_SEGS][2] = heightFactor * rad;
#endif

	basePoints[CONE_SEGS+1][0] = 0.0;
	basePoints[CONE_SEGS+1][1] = 0.0;
	basePoints[CONE_SEGS+1][2] = float(-50.0);
	
	for ( i = 0; i < CONE_SEGS; i++) // nStrips
	{
		deg = float(i*360)/float(CONE_SEGS);
		x = rad*float(sin(deg*PI/float(180.0)));
		y = rad*float(cos(deg*PI/float(180.0)));
		z = -heightFactor * rad;
		basePoints[i][0] = x;
		basePoints[i][1] = y;
		basePoints[i][2] = z;
	}

	for ( i = 0; i < (CONE_SEGS-1); i++) // nStrips
	{
		CalcNormal(basePoints[CONE_SEGS],basePoints[i],basePoints[i+1],normal);

/*		coneNormals[i].nx = (GLshort)(normal[0]*32767);
		coneNormals[i].ny = (GLshort)(normal[1]*32767);
		coneNormals[i].nz = (GLshort)(normal[2]*32767);*/
		coneNormals[i][X] = (GLshort)(normal[0]*32767);
		coneNormals[i][Y] = (GLshort)(normal[1]*32767);
		coneNormals[i][Z] = (GLshort)(normal[2]*32767);
	
	}
	// now do the last one
	CalcNormal(basePoints[CONE_SEGS],basePoints[CONE_SEGS-1],basePoints[0],normal);
/*	coneNormals[CONE_SEGS-1].nx = (GLshort)(normal[0]*32767);
	coneNormals[CONE_SEGS-1].ny = (GLshort)(normal[1]*32767);
	coneNormals[CONE_SEGS-1].nz = (GLshort)(normal[2]*32767);*/
	coneNormals[CONE_SEGS-1][X] = (GLshort)(normal[0]*32767);
	coneNormals[CONE_SEGS-1][Y] = (GLshort)(normal[1]*32767);
	coneNormals[CONE_SEGS-1][Z] = (GLshort)(normal[2]*32767);
}


