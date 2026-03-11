// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Boundary.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class Boundary;

#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "CompositeObject.h"
/** 
 *	This class provides a way to detect the crossing of a 2D or 3D boundary
 *	by one or more PosableStateBase.  Position, orientation, and scale are
 *	fixed with respect to the boundary and do not update with respect to
 *	any of the managed PosableStateBases.
 *	This class implements the following responsibilities.
 *	\li Managing PosableStateBases by collecting change events
 *		and triggering events when any one of them crosses the boundary.
 *	\li Rendering to the screen (renders a wire or wire box, if visible)
 *	\li Loading and saving from/to disk
 *
 *	\author Matt Plumlee (mdp)
 *  \date 10/9/2002
 */
class Boundary: public CompositeObject
{
  public:
	Boundary(void *creator = NULL);
	virtual ~Boundary();
	DERIVE_OBJECT(Boundary);

	/*---File-related---*/
	virtual int readFrom(FILE *file);
	virtual int processField(int option, char **fields);
	virtual int writeTo(FILE *file);

	/*---Interaction facilitation---*/
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
	virtual long manage(PosableStateBase *pose); //assigns operands

	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

	/*---Rendering---*/
	virtual void DrawLowRes(float degradation, Window3D *win);
	virtual int DrawHighRes(int eye, Window3D *win); //return 1 if done, 0 if not done.

	/*---Event notification---*/
	static unsigned int BoundaryCrossedIn() { return boundaryCrossedInInputType->value;}
	static unsigned int BoundaryCrossedOut() { return boundaryCrossedOutInputType->value;}

  protected:
	GLboolean readingFile;
	int *regionRecord, numRegionRecords;
	float drawingHeight;
	float sinYaw, cosYaw, sinPitch, cosPitch;

	Object *inWhenAbove;
	GLboolean inWhenAboveNotFlag;

	static const EventType *boundaryCrossedInInputType;
	static const EventType *boundaryCrossedOutInputType;

	enum BoundaryParams {SCALE = FirstChildKeyword, HEIGHT_RATIO, YAW, PITCH, ROLL};

	void evaluatePose(int index = -1);
	int evaluatePose(PosableStateBase *whichPose);
};

#endif
