// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// PoseState.h

class PoseState;

#ifndef POSE_STATE_H
#define POSE_STATE_H

#include "engine/PosableStateBase.h"
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, but don't have any focus or context info.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/22/2000
 */
/*---------------------------------------------------------------------------*/
class PoseState : public PosableStateBase
{
    public:
	/*---Constructors/Destructors---*/
        PoseState(PosableStateUser *initialUser = NULL, std::string name = "", OrientationInfo::Type orientationType = OrientationInfo::typeYawPitchRoll);
	PoseState(PosableStateBase *toBeReplaced);
	virtual ~PoseState();

	/*---Accessors---*/
	virtual int supportMask(int changeMask = ChangeAll);

	/*---Mutators---*/
	virtual void unsetPoseInfo(long changeMask = ChangeAll);

  protected:
	PosableStateInfo pose;

	virtual PosableStateInfo *pointerForMask(long changeMask);
};

#endif
