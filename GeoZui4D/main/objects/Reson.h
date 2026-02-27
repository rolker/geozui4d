/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef RESON_H
#define RESON_H

#include "engine/Object.h"
#include "objects/SDV_Quadtree.h"
#include "objects/nodeTypes/XYZPoint.h"

class Window3D;

class Reson : public Object
{
public:
  Reson();
	DERIVE_OBJECT(Reson);

	int Load();
	int readFrom(FILE * file) {return 1;};
	void LoadNext();
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	virtual void Idle(double time);    
private:
	SDV_Quadtree * quadtree;

	FILE * infile;
	int count;
	int elementnum;
	int doneloading;
	int deg;
	double starttime, endtime;
	Point3d geoRef;
};

#endif
