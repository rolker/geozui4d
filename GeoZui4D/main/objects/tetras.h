/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
////////////////////////////////////////////////////////////////
//
// tetras.h:  draws an object from polygon strips
// (copied from points.h for test purposes only)
//////////////////////////////////////////////////////////////////////

#ifndef TETRAS_H
#define TETRAS_H

#include "engine/Object.h"
//#include <GL/glut.h>

//#define CONE_SEGS	32
#define CONE_SEGS	3


class Tetras : public Object
{
public:

	int readFrom(FILE *file);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	virtual void ResetHighRes(); //mdp 9/5/2000

	Tetras();
	~Tetras();
	DERIVE_OBJECT(Tetras);

protected:	
	int pointCount;
	int degradation;

	Point3f * cone; // to store the cone vertices
	vNorms * coneNormals; // store the cone normals
	
	Point3f * positions;
	int * colors;

	int nPoints;
	
//wsl 2/9/01
//	Point3f palette[6];  // the color of each point;
	Point3f palette[64];  // the color of each point;


//wsl 3/16/01
	char bm2[128*128*3];
	char bm3[64][64][3];

private:
	void makeCone();
	void CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result);
	void DrawCone();
	void DrawLoresCone();

//	bool LoadBitmapFromBMPFile(LPTSTR szFileName,
//					HBITMAP *phBitmap,
//					HPALETTE *phPalette);
//	void makeCheckImage(void);

};

#endif 
