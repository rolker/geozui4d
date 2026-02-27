/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
////////////////////////////////////////////////////////////////
//
// tube.h: object that makes a polygon tube
//
//////////////////////////////////////////////////////////////////////

#ifndef TUBE_H
#define TUBE_H

#include "engine/Object.h"


class Tube : public Object
{
public:

	int readFrom(FILE *file);
	virtual int processField(int option, char **Field);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	void ResetHighRes();

	Tube();
	~Tube();
	DERIVE_OBJECT(Tube);

protected:
	int drawseg;
	int nParms;  // the number of parameters after the position
	float *size;

	int nFacets;
	int nPoints;  // the number of segments to the tube
	GLboolean spline; //spline created from points in file?
	GLboolean drawLines; // should the center line be drawn as well (to make visible from far away)?

	std::vector<gz4d::Point<double> > Path;
	Point3f * * discs; // the tube cross sections
	vNorms * * norms_array; // store the normals
	Point3f * Colors;

	float maxDiameter, minDiameter;

	Point3f Color1;
	Point3f Color2;

	virtual void CalcAllNormals();
	int CalcNormal(Point3f v1, Point3f v2, Point3f result);
	void MakeSpline();
	void DrawShadedRing(int seg, int gap);
	void CalculateCircles(float *&circX, float *&circY);

	enum TubeParams {N_RECORDS = FirstObjectKeyword, SPLINE, NO_SPLINE, DIA,
					 COLOR, XYZ};

};

#endif 
