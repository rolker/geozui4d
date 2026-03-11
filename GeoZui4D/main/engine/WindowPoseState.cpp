// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// WindowPoseState.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, but don't have any focus or context info.
 *	GL transformations are specific to windows (vs. objects)
 *
 *	Created by mdp 12/27/2000
 */
/*---------------------------------------------------------------------------*/
#include "engine/WindowPoseState.h"
#include "widgets/Sv_Standards.h"
#include "PosableStateUser.h"
#include "engine/ViewManager.h"
#include <assert.h>
#include <iostream>

using namespace std;

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
/**
\param parent The ViewManger instance for which this PoseState applies.
\param name Name to be used in the name hierarchy
\param orientationType Defaults to typeYawPitchRoll
*/
WindowPoseState::WindowPoseState(ViewManager *parent, string name, OrientationInfo::Type orientationType)
:PoseState(parent, name, orientationType)
{
	parentView = parent;
	unsetPoseInfo();

  minPose.position[X] = 0.0;
  minPose.position[Y] = 0.0;
  minPose.position[Z] = 0.0;
  minPose.scale = 1.0;

  maxPose.position[X] = 0.0;
  maxPose.position[Y] = 0.0;
  maxPose.position[Z] = 0.0;
  maxPose.scale = 1.0;

  homePose.position[X] = 0.0;
  homePose.position[Y] = 0.0;
  homePose.position[Z] = 0.0;
  homePose.scale = 1.0;

  maxPose.position[X] = 0;
  maxPose.position[Y] = 0;
  maxPose.position[Z] = 0;

  homePose.position[X] = 0;
  homePose.position[Y] = 0;
  homePose.position[Z] = 0;
}

/*-------------------------------Accessors-----------------------------------*/
/*---------------------------------------------------------------------------*/
/**	supportMask returns the portion of the mask that is supported by this
 *	posable.  Returns ChangeNone if nothing of the type passed is supported.
 */
int WindowPoseState::supportMask(int changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return (changeMask & ChangePose);
	else if (changeMask & ChangeHomeMask)
		return (changeMask & ChangeHomePose);
	else if (changeMask & ChangeMaxMask)
		return (changeMask & ChangeMaxPose);
	else if (changeMask & ChangeMinMask)
		return (changeMask & ChangeMinPose);
	else
		return ChangeNone;
}

/*-----------------------------------Toolbox---------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This is an internal function that can be used by each derived class to
 *	take advantage of the versions of the functions written here (which
 *	no child necessarily has to use).
 */
PosableStateInfo *WindowPoseState::pointerForMask(long changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return &pose;
	else if (changeMask & ChangeHomeMask)
		return &homePose;
	else if (changeMask & ChangeMaxMask)
		return &maxPose;
	else if (changeMask & ChangeMinMask)
		return &minPose;
	else
		return PoseState::pointerForMask(changeMask);
}

/*---------------------------------------------------------------------------*/
// Documented in PosableStateBase
void WindowPoseState::NotifyPose(long changeMask)
{
	long minChangeMask, maxChangeMask;

	if ((changeMask & ChangeAnyMask) == ChangeNone)
		return;

	//turn on the bits for any new min's, max's, or home.
	if (!unsetting)
	{
		switch (changeMask & ChangeTypeMask)
		{
			case ChangeMinMask:
				minMask |= (changeMask & ChangeAnyMask);
				break;
			case ChangeMaxMask:
				maxMask |= (changeMask & ChangeAnyMask);
				break;
			case ChangeHomeMask:
				homeMask |= (changeMask & ChangeAnyMask);
				break;
		}
	}

	if (!accumulatingChanges)
	{
		minChangeMask = minMask & changeMask;
		maxChangeMask = maxMask & changeMask;

		if (minChangeMask & ChangeTranslateXMask)
		{
			if (pose.position[X] < minPose.position[X])
				pose.position[X] = minPose.position[X];
		}
		if (maxChangeMask & ChangeTranslateXMask)
		{
			if (pose.position[X] > maxPose.position[X])
				pose.position[X] = maxPose.position[X];
		}
		if (minChangeMask & ChangeTranslateYMask)
		{
			if (pose.position[Y] < minPose.position[Y])
				pose.position[Y] = minPose.position[Y];
		}
		if (maxChangeMask & ChangeTranslateYMask)
		{
			if (pose.position[Y] > maxPose.position[Y])
				pose.position[Y] = maxPose.position[Y];
		}
		if (minChangeMask & ChangeTranslateZMask)
		{
			if (pose.position[Z] < minPose.position[Z])
				pose.position[Z] = minPose.position[Z];
		}
		if (maxChangeMask & ChangeTranslateZMask)
		{
			if (pose.position[Z] > maxPose.position[Z])
				pose.position[Z] = maxPose.position[Z];
		}

		if (minChangeMask & ChangeRotatePitchMask)
		{
			if (pose.orientation.axis[Pitch] < minPose.orientation.axis[Pitch])
				pose.orientation.axis[Pitch] = minPose.orientation.axis[Pitch];
		}
		if (maxChangeMask & ChangeRotatePitchMask)
		{
			if (pose.orientation.axis[Pitch] > maxPose.orientation.axis[Pitch])
				pose.orientation.axis[Pitch] = maxPose.orientation.axis[Pitch];
		}
		if (minChangeMask & ChangeRotateYawMask)
		{
			if (pose.orientation.axis[Yaw] < minPose.orientation.axis[Yaw])
				pose.orientation.axis[Yaw] = minPose.orientation.axis[Yaw];
		}
		if (maxChangeMask & ChangeRotateYawMask)
		{
			if (pose.orientation.axis[Yaw] > maxPose.orientation.axis[Yaw])
				pose.orientation.axis[Yaw] = maxPose.orientation.axis[Yaw];
		}
		if (minChangeMask & ChangeRotateRollMask)
		{
			if (pose.orientation.axis[Roll] < minPose.orientation.axis[Roll])
				pose.orientation.axis[Roll] = minPose.orientation.axis[Roll];
		}
		if (maxChangeMask & ChangeRotateRollMask)
		{
			if (pose.orientation.axis[Roll] > maxPose.orientation.axis[Roll])
				pose.orientation.axis[Roll] = maxPose.orientation.axis[Roll];
		}

		if (minChangeMask & ChangeScaleMask)
		{
			if (pose.scale < minPose.scale)
				pose.scale = minPose.scale;
		}
		if (maxChangeMask & ChangeScaleMask)
		{
			if (pose.scale > maxPose.scale)
				pose.scale = maxPose.scale;
		}
	}

	PosableStateBase::NotifyPose(changeMask);
}

/*------------------------------------Mutators-------------------------------*/
/*---------------------------------------------------------------------------*/
/**	unsetPoseInfo resets position, orientation, and/or scale information,
 *	depending on the passed changeMask, if supported.  Reseting using
 *	ChangeAll (the default) actually resets all values, including the
 *	min, max, and preferred values for the selected attributes (position,
 *	orientation, scale).
 */
void WindowPoseState::unsetPoseInfo(long changeMask)
{
	if (changeMask & ChangeMinMask)
		minMask &= ~changeMask;
	if (changeMask & ChangeMaxMask)
		maxMask &= ~changeMask;
	if (changeMask & ChangeHomeMask)
		homeMask &= ~changeMask;

	//handle the regular part.
	unsetting = GL_TRUE;
	PoseState::unsetPoseInfo(changeMask);
	unsetting = GL_FALSE;
}

/*-----------------------------GL Transformations----------------------------*/
/*---------------------------------------------------------------------------*/
/**
Applies rotation and scale, and transforms the coordinate system.
Does not translate, as our location is OpenGL's 0,0,0. The axis are
also rotated and flipped so that +x is east, +y is north and +z is up.
*/
void WindowPoseState::applyTransforms(ViewManager * view)
{
	glScaled(pose.scale, pose.scale, pose.scale);
	if (pose.orientation.type == OrientationInfo::typeYawPitchRoll)
	{
		glRotated(pose.orientation.axis[Roll], 0.0, 0.0, 1.0);
		glRotated(pose.orientation.axis[Pitch], 1.0, 0.0, 0.0);
		glRotated(pose.orientation.axis[Yaw], 0.0, 1.0, 0.0);
	}
	else if (pose.orientation.type == OrientationInfo::typePseudoQuaternion)
		glRotated(pose.orientation.angle,
					pose.orientation.axis[X],
					pose.orientation.axis[Y],
					pose.orientation.axis[Z]);
	else //typeQuaternion
		assert(0);//not implemented yet

	//glTranslated(-pose.position[X], parentView->getHeightExaggeration() * -pose.position[Z], pose.position[Y]);

	glRotated(90.0, 1.0, 0.0, 0.0); // apply a rotation to the whole scene
	glScaled(1.0, -1.0, -1.0);     // invert y & z
}


/*--------------------Static PosableStateInfo Operations---------------------*/
/*---------------------------------------------------------------------------*/
/** This function returns the ratio of the distance of the passed "object"
 *	location from the passed view location, normalized with respect to the
 *	size of the view's workspace-center (as a shpere).
 */
float WindowPoseState::viewToObjectPoseInfo(const PosableStateInfo &view,
											const PosableStateInfo &object)
{
	float result = 0.0, val;
	int dim;

	for (dim = X; dim <= Z; dim++)
	{
		val = object.position[dim] - view.position[dim];
		result += val * val;
	}
	result = (float) (view.scale * sqrt(result) / ViewManager::radiusOfWorkspaceCenter);
	return result;
}
