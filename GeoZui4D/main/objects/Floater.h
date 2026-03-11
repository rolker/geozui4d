// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
