// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// S57_landmark.h
/*---------------------------------------------------------------------------*/
/** 
 *	This object is used to visualize S57 landmarks (LNDMRK)
 *	
 *	Created by mdp 3/8/2005
 */
/*---------------------------------------------------------------------------*/
#ifndef S57_LANDMARK_H
#define S57_LANDMARK_H

#include"S57Layer.h"
#include"S57_beacon.h"

class S57_landmark : public S57_beacon //changed to derive from S57_beacon 8/17/2009 by mdp
{
public:
	S57_landmark();
	~S57_landmark();
	DERIVE_OBJECT(S57_landmark);

	virtual void SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber);
	void DrawLowRes(float degradation,Window3D * win);
    virtual void ShowS57ShapeInfo(Scene3D *scene);

	static char const *functions[];
	static char const *landmarkCategories[];

private:
	int function, category, printOffset;
	char *displayLabel;
};

#endif
