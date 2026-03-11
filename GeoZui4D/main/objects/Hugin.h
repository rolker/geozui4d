// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SampleObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation, for use as a template in creating
 *	new objects
 *
 *	Created by mdp 12/13/2000
 */
/*---------------------------------------------------------------------------*/
class Hugin;

#ifndef HUGIN_H
#define HUGIN_H

#include "engine/Object.h"
#include "interactors/Pickable.h"
#include "engine/Tail.h"

class PosableStateLock;

class Hugin : public Object
{
  public:
    Hugin();
	DERIVE_OBJECT(Hugin);

	//File-related functions
    int readFrom(FILE *file); //read from a file
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);

	//Picking functions
    virtual int  pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

	//Update functions
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

  protected:
	//private data members go here
	Tail tail;
//	Point3f oldPosition;
};

#endif
