// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// triangles.h:  draws an object from polygon strips
//
//////////////////////////////////////////////////////////////////////

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "engine/Object.h"


class Triangles : public Object
{
public:

	int readFrom(FILE *file);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	void ResetHighRes();

	Triangles();
	~Triangles();
	DERIVE_OBJECT(Triangles);

protected:	
	int drawrow;
//	int degradation;

	Point3f * * strips; // to store the vertices

	int nStrips;
	
	int * stripLengths;  // the length of each triangle strip
	
	vNorms * * norms_array; // store the normals
	

private:
	void CalcAllNormals();
	void CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result);
	void DrawShadedStrip(int row, int gap);

};

#endif 
