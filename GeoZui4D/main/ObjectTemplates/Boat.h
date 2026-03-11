// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Boat.h
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation that shows off several common
 *	techniques used to derive a GeoZui object.
 *
 *	Created by mdp 5/4/2005
 */
/*---------------------------------------------------------------------------*/
class Boat;

#ifndef BOAT_H
#define BOAT_H

#include "engine/Object.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"

class PosableStateLock;

class Boat: public Object
{
    public:
	Boat();
	~Boat();
	DERIVE_OBJECT(Boat);

	//Geometry-related functions
	enum {BOW, HULL, STERN, BRIDGE, NUM_PARTS};
	void calculateOffsets(float bridge_nudge = 0.0f);

	//Info functions
	virtual void ShowInfo(unsigned int *names = NULL, int numNames = 0);

	//File-related functions
	int readFrom(FILE *file); //read from a file
	int processField(int option, char **fields);
	Object * LoadXML(bpt::ptree const &xml, char * path, gz4d::netcdf::File ncfile);

	//Display-related functions
	void DrawLowRes(float degradation, Window3D *win);

	//Update functions
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

	//Scripting interface
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

protected:
	//private data members go here
	Tail tail;
	bool drawTail, drawCMG;
	float width[NUM_PARTS], length[NUM_PARTS], height[NUM_PARTS], offset[NUM_PARTS];
	int shadow;
	double heading;

	//keep these so we can look at how another vessel sees the world
	PosableStateBase *courseMadeGood;
	PosableStateLock *lockFromCMG;

	virtual void initializingLabel();

	enum BoatParams {BOW_SIZE = FirstObjectKeyword, HULL_SIZE,
		   STERN_SIZE, BRIDGE_SIZE, SHADOW, FirstBoatKeyword};
};

#define BOAT_NAMES \
	OBJECT_NAMES, "BOW_SIZE", "HULL_SIZE", \
	"STERN_SIZE", "BRIDGE_SIZE", "SHADOW"
#endif
