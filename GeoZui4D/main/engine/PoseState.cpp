/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PoseState.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, orientation,
 *	and scale properties, but don't have any focus or context info.
 *
 *	Created by mdp 12/22/2000
 */
/*---------------------------------------------------------------------------*/
#include "PoseState.h"
#include "widgets/Sv_Standards.h"
#include "PosableStateUser.h"
#include "engine/Scene3D.h"

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
/**
\param initialUser Defaults to NULL
\param name Defaults to NULL
\param orientationType Specifies how OrientationInfo encodes its information.
Defaults to typeYawPitchRoll
*/
PoseState::PoseState(PosableStateUser *initialUser, std::string name, OrientationInfo::Type orientationType)
    :PosableStateBase(initialUser, name)
{
	pose.orientation.type = orientationType;
	pose.position[X] = 0.0;
	pose.position[Y] = 0.0;
	pose.position[Z] = 0.0;

        if (orientationType == OrientationInfo::typeQuaternion)
                pose.orientation.angle = 1.0;
        else
                pose.orientation.angle = 0.0;

	pose.scale = 1.0;
}

/*---------------------------------------------------------------------------*/
PoseState::PoseState(PosableStateBase *toBeReplaced)
:PosableStateBase(toBeReplaced)
{
	pose.orientation.type = OrientationInfo::typeYawPitchRoll;
	pose.position[X] = 0.0;
	pose.position[Y] = 0.0;
	pose.position[Z] = 0.0;

	pose.orientation.axis[Yaw] = 0.0;
	pose.orientation.axis[Pitch] = 0.0;
	pose.orientation.axis[Roll] = 0.0;

	pose.scale = 1.0;
}

/*---------------------------------------------------------------------------*/
PoseState::~PoseState()
{
}

/*-------------------------------Accessors-----------------------------------*/
/*---------------------------------------------------------------------------*/
/**	supportMask returns the portion of the mask that is supported by this
 *	posable.  Returns ChangeNone if nothing of the type passed is supported.
 */
int PoseState::supportMask(int changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return (changeMask & ChangePose);
	else
		return ChangeNone;
}

/*-----------------------------------Toolbox---------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This is an internal function that can be used by each derived class to
 *	take advantage of the versions of the functions written here (which
 *	no child necessarily has to use).
 */
PosableStateInfo *PoseState::pointerForMask(long changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return &pose;
	else
		return PosableStateBase::pointerForMask(changeMask);
}

/*------------------------------------Mutators-------------------------------*/
/*---------------------------------------------------------------------------*/
/**	unsetPoseInfo resets position, orientation, and/or scale information,
 *	depending on the passed changeMask, if supported.  Reseting using
 *	ChangeAll (the default) actually resets all values, including the
 *	min, max, and preferred values for the selected attributes (position,
 *	orientation, scale).
 */
void PoseState::unsetPoseInfo(long changeMask)
{
	if (changeMask & ChangeCurrentMask)
	{
		if (changeMask & ChangeTranslateMask)
		{
			pose.position[X] = 0.0;
			pose.position[Y] = 0.0;
			pose.position[Z] = 0.0;
		}

		if (changeMask & ChangeRotateMask)
		{
                    if (pose.orientation.type == OrientationInfo::typeYawPitchRoll)
			{
				pose.orientation.axis[Yaw] = 0.0;
				pose.orientation.axis[Pitch] = 0.0;
				pose.orientation.axis[Roll] = 0.0;
			}
			else
			{
				pose.orientation.axis[X] = 0.0;
				pose.orientation.axis[Y] = 0.0;
				pose.orientation.axis[Z] = 0.0;
                                if (pose.orientation.type == OrientationInfo::typeQuaternion)
					pose.orientation.angle = 1.0;
				else
					pose.orientation.angle = 0.0;
			}
		}

		if (changeMask & ChangeScaleMask)
			pose.scale = 1.0;
	}
	NotifyPose(changeMask);
}
