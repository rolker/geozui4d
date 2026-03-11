// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SampleObject.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class SampleObject;

#ifndef SAMPLEOBJECT_H
#define SAMPLEOBJECT_H

#define SMOOTH_AMOUNT 10

#include "engine/Object.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"

class PosableStateLock;
/** 
 *	This is a sample object instantiation, for use as a template in creating
 *	new objects
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/13/2000
 */
class SampleObject : public Object
{
  public:
    SampleObject();
	DERIVE_OBJECT(SampleObject);

	//File-related functions
    int readFrom(FILE *file); //read from a file
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);

	//Update functions
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

  protected:
	//private data members go here
	Tail tail;
	Point3d oldPosition[SMOOTH_AMOUNT];
};

#endif
