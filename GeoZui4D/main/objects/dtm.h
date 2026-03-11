// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
////////////////////////////////////////////////////////////////
//
// dtm.h: interface for the dtm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DTM_H
#define DTM_H

#include "engine/Object.h"

class Dtm : public Object
{
public:

	//File-related
	int readFrom(FILE *file);

	//Rendering
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	void DrawShadedStrip(int row, int gap);
	virtual void ResetHighRes(); //mdp 8/3/2000

	Dtm();
	~Dtm();
	DERIVE_OBJECT(Dtm);

protected:

/*	struct vect
	{
		GLfloat x,y,z;
	};*/
	
	int drawrow;
	void CalculateNormals();

	GLfloat * * dem; // to store the actual height values


//	GLubyte red[256];
//	GLubyte green[256];
//	GLubyte blue[256];

	float maxy;
	float maxx;
	
	float yres;
	float xres;
	float zres;

	float heightscale;
	

	vNorms * * norms_array;
	
	float maxheight;
	float minheight;

	int eye;
};

#endif 
