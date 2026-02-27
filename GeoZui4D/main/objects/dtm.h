/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
