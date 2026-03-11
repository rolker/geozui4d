// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include <math.h>
#include "objects/GeoData/Cylinder.h"
#include <iostream>
#include "engine/ToglWindow.h"

using namespace std;

#ifdef EXPERIMENT_GLYPHS
#define N_SEGS	10
#define H_FACT 0.5f
#else
#define N_SEGS	30
#define H_FACT 1.0f
#endif
#define LO_RES_STEP 6
#define RADIUS	1.4f
#define PI	float(3.141592)

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
Cylinder::Cylinder()
{
	Normals = new vNorms[N_SEGS*2];
	Points = new Point3f[N_SEGS*2];

	radius = RADIUS;
	heightFactor = H_FACT;
	orientation = 0.0;
	makeCylinder();
}

/*--------------------------------------------------------------------------*/
Cylinder::Cylinder(float rad)
{
	Normals = new vNorms[N_SEGS*2];
	Points = new Point3f[N_SEGS*2];

	radius = rad;
	heightFactor = H_FACT;
	orientation = 0.0;
	makeCylinder();
}

/*--------------------------------------------------------------------------*/
Cylinder::Cylinder(float rad, float hgtFactor, float orient)
{
	Normals = new vNorms[N_SEGS*2];
	Points = new Point3f[N_SEGS*2];

	radius = rad;
	heightFactor = hgtFactor;
	orientation = orient;
	makeCylinder();
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Cylinder::draw(float deg, int renderMode)
{

	int i,step;

	step = 2;
	if (deg > 0.1) step = LO_RES_STEP*2;

	glShadeModel(GL_SMOOTH);
#ifdef EXPERIMENT_GLYPHS
	glRotatef(65, -1.0, 0.5, 0);
#else
	if (orientation != 0.0f)
		glRotatef(orientation, 1.0, 0.0, 0.0);
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

	//do pairs of points
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < N_SEGS*2; i+=step)
	{
		//glNormal3s(Normals[i].nx, Normals[i].ny, Normals[i].nz);
		glNormal3sv(Normals[i]);
		glVertex3fv(Points[i]);
		glVertex3fv(Points[i+1]);
	}
	//glNormal3s(Normals[0].nx, Normals[0].ny, Normals[0].nz);
	glNormal3sv(Normals[0]);
	glVertex3fv(Points[0]);
	//glNormal3s(Normals[1].nx, Normals[1].ny, Normals[1].nz);
	glNormal3sv(Normals[1]);
	glVertex3fv(Points[1]);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(0.0,0.0,radius);
	for (i = 1; i < N_SEGS*2; i+=step)
	{	
		glVertex3fv(Points[i]);
	}
	glVertex3fv(Points[1]);
	glEnd();
}

/*--------------------------------------------------------------------------*/
void Cylinder::makeCylinder()
{
	int i;
	float x,y,angle;

	for ( i = 0; i < N_SEGS; i++) // nStrips
	{
		angle = float(i)/float(N_SEGS);
		x = float(sin(2.0*angle*PI));
		y = float(cos(2.0*angle*PI));
		Points[2*i][0] = x*radius;
		Points[2*i][1] = y*radius;
		Points[2*i][2] = - radius * heightFactor;
		Points[2*i+1][0] = x*radius;
		Points[2*i+1][1] = y*radius;
		Points[2*i+1][2] = radius * heightFactor;

/*		Normals[2*i].nx = (GLshort)(x*32767);
		Normals[2*i].ny = (GLshort)(y*32767);
		Normals[2*i].nz = (GLshort)(0);
		Normals[2*i+1].nx = (GLshort)(x*32767);
		Normals[2*i+1].ny = (GLshort)(y*32767);
		Normals[2*i+1].nz = (GLshort)(0);*/
		Normals[2*i][X] = (GLshort)(x*32767);
		Normals[2*i][Y] = (GLshort)(y*32767);
		Normals[2*i][Z] = (GLshort)(0);
		Normals[2*i+1][X] = (GLshort)(x*32767);
		Normals[2*i+1][Y] = (GLshort)(y*32767);
		Normals[2*i+1][Z] = (GLshort)(0);

	}
}


