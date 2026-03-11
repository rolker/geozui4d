// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// AveragingOverview.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class AveragingOverview;

#ifndef AVERAGINGOVERVIEW_H
#define AVERAGINGOVERVIEW_H

#include "CompositeObject.h"
/** 
 *	This class provides a way to represent an average of multiple reference
 *	frames in their position and rotation attributes.  Scale is updated such
 *	that all positions would be present in the center of workspace of
 *	any ViewManager attached to this (hence the Overview portion).
 *	This class implements the following responsibilities.
 *	\li Managing PosableStateBases by collecting change events
 *	    and keeping the result pose updated.
 *	\li Rendering to the screen (renders a wire box, if visible)
 *	\li Loading and saving from/to disk
 *
 *	\author Matt Plumlee (mdp)
 *  \date 6/22/2001
 */
class AveragingOverview: public CompositeObject
{
  public:
	AveragingOverview(void *creator = NULL);
	virtual ~AveragingOverview();
	DERIVE_OBJECT(AveragingOverview);

	/*---File-related---*/
	virtual int processField(int option, char **fields);
	virtual int writeTo(FILE *file);

	/*---Interaction facilitation---*/
	virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);
	virtual long manage(PosableStateBase *pose); //assigns operands
	virtual void relinquish(PosableStateBase *pose = NULL); //relinquish all operands if NULL
	void enable(long changeMask);
	void disable(long changeMask);

	/*---Rendering---*/
	virtual void DrawLowRes(float degradation, Window3D *win);
	virtual int DrawHighRes(int eye, Window3D *win); //return 1 if done, 0 if not done.

  protected:
	enum AvovParams {TRACKING = FirstChildKeyword, AVERAGE, MEAN, MIN_MAX};
	Point3d minPos, maxPos;
	PosableStateBase *minPose[3], *maxPose[3];
	float distance;
	GLboolean averageMean, updatingScale;

	void updateScale(PosableStateBase *changedPose, GLboolean leaving = GL_FALSE);
};

#endif
