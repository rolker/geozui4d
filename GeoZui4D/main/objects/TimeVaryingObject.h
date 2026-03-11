// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// TimeVaryingObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This is a generalized mechanism for creating time-varying objects of any
 *	sort, assuming they know how to load themselves from netCDF.  This object
 *	basically keeps track of a bunch of different versions of the same object,
 *	rendering the appropriate one for the current GeoZui4D time.
 *
 *	Created by mdp 2/11/2004 (by smushing Roland's TVGutm and Tide classes together)
 */
/*---------------------------------------------------------------------------*/
#ifndef TIMEVARYINGOBJECT_H
#define TIMEVARYINGOBJECT_H

#include"engine/ObjectCollection.h"

class TimeVaryingObject: public ObjectCollection
{
public:

	DERIVE_OBJECT(TimeVaryingObject);
	
	TimeVaryingObject();
	~TimeVaryingObject();
	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
	virtual void Idle(double time);
	void DrawLowRes(float degradation, Window3D * win);
	int DrawHighRes(int eye, Window3D * win);
	void Fuse(Object *other){}; //I'm not sure if we really want to do this, but ok for newly created class...
	double GetHeightAt(double x, double y, Point3d coord = NULL, GLboolean secondary = GL_FALSE);

	Object * GetCurrent(double time);
	Object * GetOffsetFromCurrent(double timeOffset);
	Object * GetCurrent() {return currentObject;}
	//double GetTime() {return currentTime;}

protected:
	virtual void initializeAs(const char* alias);
	std::string defaultLoadingType;
	int treatAsTide;
	void sortAndIndexByTime();
	void extractNode(Node *);

private:
	Object * currentObject;
	Node **timeIndex;
	int numObjects;
	double indexStep, timeOffset;
	double currentGzTime;
	double timeDistance(BBox b,double t);
};

#endif
