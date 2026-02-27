/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef FLOATER_H
#define FLOATER_H

#include"engine/Object.h"

class CurrentLayer;

class Floater : public Object
{
public:
	Floater();
	~Floater();
	DERIVE_OBJECT(Floater);
	void DrawLowRes(float degradation, Window3D * win);
	void setCurrent(CurrentLayer * c,double time);
	void setTailLength(int tl);
	void Idle(double time);

	//attribute stuff
	int getAttribute(SmartValue &val, int attributeIndex);
	int setAttribute(SmartValue &val, int attributeIndex);

private:
	Point3d * positions;
	int tailLength;
	double timestep;

	CurrentLayer * current;

	enum FloaterAttributes {Attr_TailLength = NumObjectAttributes, NumFloaterAttributes};

	
};

#endif
