/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// tail.h
/*---------------------------------------------------------------------------*/

class Tail;

#ifndef TAIL_H
#define TAIL_H

#include<gz4d/glo.h>
#include "SDV_types.h"
#include <gz4d/Point.h>

class PosableStateBase;

class Window3D;
/** 
 *	object that makes a tube-like tail.  just keep adding to the head.
 *
 *	\author Colin Ware
 *  \date 2000
 */
/*---------------------------------------------------------------------------*/
class Tail
{
public:
	//Data updating
	void ResetTail();
	void newPosition(gz4d::Point<double> const &p);
	void truncateTailAt(int index = -1);

	//Data querying
	float getAverageCourse(int howFarBack, PosableStateBase *pose);

	//Rendering
	void DrawIn(Window3D *win);

	Tail(int length, float dia);
	~Tail();

protected:

	int nFacets;

	//int pathpos;  // the position of the head
	int tailpos; // the position in the tail
	int taillength;  // the length of the tail starts at 0
	int truncateLength;
	int Maxtail; // the length it grows to
	float * circX;  // we store a circle to speed calculation later
	float * circY;

	Point3d * * discs; // the tube cross sections
	vNorms * * norms_array; // store the normals
	Point3f * Colors;

        gz4d::Point<double> Normal, Side, Forward;
        gz4d::Point<double> oldPath, oldoldPath;

        gz4d::Point<double> pos; //offset position of window being rendered to

	float maxDiameter, minDiameter;

        GLboolean CalcNormals(int i, gz4d::Point<double> const &path);
        int CalcNormal(gz4d::Point<double> const &v1, gz4d::Point<double> const &v2, gz4d::Point<double> &result);
	void DrawShadedRing(int seg, int gap);
	void CalculateCircles();
};

#endif 
