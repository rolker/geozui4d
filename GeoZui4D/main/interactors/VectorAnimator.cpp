// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// VectorAnimator.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	Instances of this class animate a PosableStateBase along a given vector.
 *	This class is responsible for the following.
 *		* Managing operand and resultant PosableStateBases
 *		* Process input events
 *		* Recognize a "cancel" event
 *
 *	Created by mdp 9/13/2002
 */
/*---------------------------------------------------------------------------*/

#include "engine/ObjectPoseState.h"
#include "VectorAnimator.h"
#include "widgets/Sv_Standards.h"
#include "engine/gzCommandInterface.h"
#include "engine/Object.h"
#include <string.h>
#include <stdlib.h>
//#include "engine/Joystick.h"

#include <iostream>

using namespace std;

REGISTER_ANIMATOR(VectorAnimator);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
VectorAnimator::VectorAnimator(PosableStateBase *resultant,
									 PosableStateBase *directionOperand)
: Animator(resultant, 2)
{
	init(directionOperand);
}

/*---------------------------------------------------------------------------*/
VectorAnimator::VectorAnimator(string name,
									 PosableStateBase *resultant,
									 PosableStateBase *directionOperand)
: Animator(resultant, name, 2)
{
	init(directionOperand);
}

/*---------------------------------------------------------------------------*/
void VectorAnimator::init(PosableStateBase *directionOperand)
{
	addOperand(directionOperand);
	addOperand(NULL);
}

/*---------------------------------------------------------------------------*/
VectorAnimator::~VectorAnimator()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int VectorAnimator::HandleInput(unsigned int inputType, InputInfo *info)
{
	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
			resultantPose->setContinuousChange(GL_FALSE);
			return PASS_EVENT_THROUGH;
		}
		return Animator::HandleInput(inputType, info);
	}

	return Animator::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
int VectorAnimator::animate()
{
	PosableStateInfo accum;
        gz4d::Point<double> orientation;
	float speed = 1, total, cosPitch, pitch, yaw;
	int dim;

	//what speed should we go?
	if (operandPoses[SpeedOperand])
		speed = operandPoses[SpeedOperand]->getScale();

	//get the new orientation
	if (operandPoses[DirectionOperand])
	{
            gz4d::Point<double> direction = operandPoses[DirectionOperand]->getPosition();
		total = 0.0;
		for (dim = X; dim <= Z; dim++)
			total += direction[dim] * direction[dim];
		if (total == 0.0f)
		{
			orientation = operandPoses[DirectionOperand]->getOrientationAxis();
			pitch = (float)(-orientation[Pitch] * M_PI / 180);
			yaw = (float)(-orientation[Yaw] * M_PI / 180);
			cosPitch = (float)cos(pitch);
			accum.position[X] = (-(float)sin(yaw)) * cosPitch * speed;
			accum.position[Y] = ((float)cos(yaw)) * cosPitch * speed;
			accum.position[Z] = ((float)sin(pitch)) * speed;
		        accum.orientation.axis = orientation;
		}
		else
		{
			if (total < 0.999 || total > 1.001)
				 speed /= (float)sqrt(total);

			//project us out
			for (dim = X; dim <= Z; dim++)
			{
				accum.position[dim] = direction[dim] * speed;
			}

			//get us oriented properly
			resultantPose->vectorToAnglePoseInfo(accum, accum);
		}
	}
	else
		return PASS_EVENT_THROUGH;

	/* set the position and direction */
	resultantPose->AccumulateNotifications();

	//position
	if (changeMask & ChangeTranslateMask)
		resultantPose->offsetPoseInfoBy(accum, ((changeMask & ChangeTranslateMask) | ChangeCurrentMask));
	//orientation
	if (changeMask & ChangeRotateMask)
	{
		while (accum.orientation.axis[Yaw] > 180.0)
			accum.orientation.axis[Yaw] -= 360.0;
		while (accum.orientation.axis[Yaw] < -180.0)
			accum.orientation.axis[Yaw] += 360.0;

		resultantPose->setOrientation(accum.orientation, changeMask);
	}

	resultantPose->ResumeSingularNotification();

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
int VectorAnimator::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	Addressable *address;
	PosableStateBase *newPose;
	int retval;
//	char retstr[16] = "";
//	GLboolean simpleFlag = GL_TRUE;

	if (argc >= 1)
	{
		if (!strcmp(argv[0], "direction"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "Usage: direction <pose>", NULL);
				return TCL_ERROR;
			}
			retval = gzCommandInterface::findPosable(argv[1], address, newPose);
			if (retval != TCL_OK)
				return retval;
			replaceOperand(newPose, DirectionOperand);
			return TCL_OK;
		}
		if (!strcmp(argv[0], "speed"))
		{
			if (argc < 2 || !strcmp(argv[1], "usage"))
			{
				Tcl_AppendResult(interp, "Usage: speed {usage | <pose>}", NULL);
				return TCL_ERROR;
			}
			retval = gzCommandInterface::findPosable(argv[1], address, newPose);
			if (retval != TCL_OK)
				return retval;
			replaceOperand(newPose, SpeedOperand);
			return TCL_OK;
		}
	}
	return Animator::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int VectorAnimator::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "speed direction ", NULL);
	return Animator::list(interp);
}
