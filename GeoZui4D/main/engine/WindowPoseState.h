// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// WindowPoseState.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class WindowPoseState;

#ifndef WINDOW_POSE_STATE_H
#define WINDOW_POSE_STATE_H

#include "PoseState.h"

class ViewManager;
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, plus minimum, maximum, and home versions of each,
 *	but don't have any focus or context info.
 *	GL transformations are specific to windows (vs. objects)
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/27/2000
 */
class WindowPoseState : public virtual PoseState
{
  public:
	/*---Constructors/Destructors---*/
      WindowPoseState(ViewManager *parent, std::string name = "", OrientationInfo::Type orientationType = OrientationInfo::typeYawPitchRoll);

	/*---Interaction facilitation---*/
	virtual void NotifyPose(long changeMask);

	/*---Accessors---*/
	virtual int supportMask(int changeMask = ChangeAll);

	/*---Mutators---*/
	virtual void unsetPoseInfo(long changeMask = ChangeAll);

	/*---GL Transformations---*/
	virtual void applyTransforms(ViewManager * view = NULL);
	

	/*---Static PosableStateInfo Operations---*/
	static float viewToObjectPoseInfo(const PosableStateInfo &view, const PosableStateInfo &object);

  protected:
	PosableStateInfo homePose;
	long homeMask;
	PosableStateInfo minPose, maxPose;
	long minMask, maxMask;
	GLboolean unsetting;

	ViewManager *parentView;

	PosableStateInfo *pointerForMask(long changeMask);
};

#endif
