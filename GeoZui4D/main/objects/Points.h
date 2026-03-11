// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// Points.h:  draws an object from polygon strips
//
//////////////////////////////////////////////////////////////////////

#ifndef POINTS_H
#define POINTS_H

#include "engine/Object.h"
//#include <GL/glut.h>

#define CONE_SEGS	32


class Points : public Object
{
public:

	int readFrom(FILE *file);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	virtual void ResetHighRes(); //mdp 9/5/2000

	virtual void ShowInfo(GLuint *names, int numNames);

	Points();
	Points(char * c);
	~Points();
	DERIVE_OBJECT(Points);

protected:	
	int pointCount;
	int degradation;
	int colflag, sizeflag, textflag; // flags for different data parms

	Point3f * cone; // to store the cone vertices
	vNorms * coneNormals; // store the cone normals
	
	Point3f * positions;
	int * colors;
	float * sizes;
	char **comments;

	int nPoints;
	
	Point3f local_palette[10];  // the color of each point;

	

private:
	void makeCone();
	void CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result);
	void DrawCone();
	void DrawLoresCone();

};

#endif 
