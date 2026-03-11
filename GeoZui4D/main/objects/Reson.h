// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
