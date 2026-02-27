/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Animator.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the interface for classes that animate one PosableStateBase to
 *	another, with some potential input from various input streams.
 *	Implementations of this interface are responsible for the following.
 *		* Managing operand and resultant PosableStateBases
 *		* Process input events
 *		* Recognize a "cancel" event
 *
 *	Created by mdp 1/17/2002
 */
/*---------------------------------------------------------------------------*/

#include "engine/ObjectPoseState.h"
#include "interactors/ImitatingAnimator.h"
#include "widgets/Sv_Standards.h"
#include "engine/gzCommandInterface.h"
#include "engine/InputMediator.h"
#include <string.h>
#include <stdlib.h>

REGISTER_ANIMATOR(ImitatingAnimator);

const EventType *ImitatingAnimator::imitationStabilizedInputType =
				Interactor::newEventType("imitationStabilized", Interactor::DataAsAddress);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ImitatingAnimator::ImitatingAnimator(PosableStateBase *resultant,
									 PosableStateBase *offsetOperand)
: Animator(resultant, 2)
{
	init(offsetOperand);
}

/*---------------------------------------------------------------------------*/
ImitatingAnimator::ImitatingAnimator(std::string name,
									 PosableStateBase *resultant,
									 PosableStateBase *offsetOperand)
: Animator(resultant, name, 2)
{
	init(offsetOperand);
}

/*---------------------------------------------------------------------------*/
void ImitatingAnimator::init(PosableStateBase *offsetOperand)
{
	addOperand(NULL);
	addOperand(offsetOperand);

	speed = 1.0;

	changedLastCycle = GL_TRUE;

	scaleTempersSpeed = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
ImitatingAnimator::~ImitatingAnimator()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int ImitatingAnimator::HandleInput(unsigned int inputType, InputInfo *info)
{
	int key;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
			resultantPose->setContinuousChange(GL_FALSE);
			return PASS_EVENT_THROUGH;
		}
		return Animator::HandleInput(inputType, info);
	}

    if (!info || !(inputType & DataAsInt))
        return Animator::HandleInput(inputType, info);

 	/*---Handle Keyboard Events---*/
	if (inputType & KeyAction)
	{
		key = info->val.i;
		if (key & KeySpecial && inputType & KeyDown)
		{
			if (key == KeyUpArrow)
			{
				speed *= 1.25f;
				return SINK_EVENT;
			}
			if (key == KeyDownArrow)
			{
				speed /= 1.25f;
				return SINK_EVENT;
			}
		}
	}

	return Animator::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
int ImitatingAnimator::animate()
{
	float distance, val, scalespeed, translateSpeed;
	PosableStateInfo accum, temp;
	long mask;
	int i;
	GLboolean makingChange = GL_FALSE;

	//accumulate operands
	if (operandPoses[BaseOperand])
	{
		operandPoses[BaseOperand]->getPoseInfo(accum, changeMask);
		if (operandPoses[OffsetOperand])
		{
			operandPoses[OffsetOperand]->getPoseInfo(temp, changeMask);
			resultantPose->incrementPoseInfo(accum, temp, changeMask);
		}
	}
	else if (operandPoses[OffsetOperand])
		operandPoses[OffsetOperand]->getPoseInfo(accum, changeMask);
	else
		return PASS_EVENT_THROUGH;

	//subtract current result to get desired delta
	resultantPose->getPoseInfo(temp, changeMask);
	resultantPose->decrementPoseInfo(accum, temp, changeMask);

	//determine portion of delta we will be moving by...
	//...translation
	if (changeMask & ChangeTranslateMask)
	{
		distance = 0.0;
		for (i = X, mask = ChangeTranslateXMask; i <= Z; i++, mask <<= 2)
		{
			if (changeMask & mask)
			{
				val = accum.position[i];
				distance += val * val;
			}
		}
		if (distance != 0.0)
		{
			distance = (float)sqrt(distance);
			if (scaleTempersSpeed)
				translateSpeed = (float)(1000 * speed / exp(temp.scale));
			else
				translateSpeed = speed;
			if (distance > translateSpeed)
				resultantPose->multiplyPoseInfo(accum, translateSpeed / distance, changeMask & ChangeTranslateMask);
			makingChange = GL_TRUE;
		}
	}

	//...rotation
	if (changeMask & ChangeRotateMask)
	{
		distance = 0.0;
		if (accum.orientation.type == OrientationInfo::typeYawPitchRoll)
		{
			for (i = Pitch, mask = ChangeRotatePitchMask; i <= Roll; i++, mask <<= 2)
			{
				if (changeMask & mask)
				{
					val = accum.orientation.axis[i];
					while (val > 180.0)
						val -= 360.0;
					while (val < -180.0)
						val += 360.0;
					accum.orientation.axis[i] = val;
					distance += val * val;
				}
			}
			distance = (float)sqrt(distance) * 10.0f;
		}
		else //&&&not really right...
			distance = accum.orientation.angle;
		if (distance != 0.0)
		{
			if (distance > speed)
				resultantPose->multiplyPoseInfo(accum, speed / distance, changeMask & ChangeRotateMask);
			makingChange = GL_TRUE;
		}
	}

	//...scale
	if (changeMask & ChangeScaleMask)
	{
		if (speed < 0)
			scalespeed = 0;
		else
			scalespeed = speed;
		scalespeed = (float)(exp(1 + scalespeed / 100.0f) / exp(1.0));
		if (accum.scale > scalespeed)
			accum.scale = scalespeed;
		else
		{
			scalespeed = 1.0f / scalespeed;
			if (accum.scale < scalespeed)
				accum.scale = scalespeed;
		}
		if (accum.scale != 1.0f)
			makingChange = GL_TRUE;
	}

	//make the change
	if (makingChange)
	{
		resultantPose->offsetPoseInfoBy(accum, changeMask);
		changedLastCycle = GL_TRUE;
	}
	else
	{
		if (changedLastCycle)
		{
			highLevelEventMediator->Trigger(imitationStabilizedInputType->value, this, resultantPose);
			changedLastCycle = GL_FALSE;
		}
	}

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
int ImitatingAnimator::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	Addressable *address;
	PosableStateBase *newPose;
	int retval;

	if (argc >= 1)
	{
		if (!strcmp(argv[0], "speed"))
		{
			if (argc < 2 || !strcmp(argv[1], "usage"))
			{
				Tcl_AppendResult(interp, "Usage: speed {<value> | usage}", NULL);
				if (!strcmp(argv[1], "usage"))
					return TCL_OK;
				else
					return TCL_ERROR;
			}
			setSpeed((float)atof(argv[1]));
			return TCL_OK;
		}
		if (!strcmp(argv[0], "base"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "Usage: base <pose>", NULL);
				return TCL_ERROR;
			}
			retval = gzCommandInterface::findPosable(argv[1], address, newPose);
			if (retval != TCL_OK)
				return retval;
			replaceOperand(newPose, BaseOperand);
			return TCL_OK;
		}
		if (!strcmp(argv[0], "offset"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "Usage: offset <pose>", NULL);
				return TCL_ERROR;
			}
			retval = gzCommandInterface::findPosable(argv[1], address, newPose);
			if (retval != TCL_OK)
				return retval;
			replaceOperand(newPose, OffsetOperand);
			return TCL_OK;
		}
		if (!strcmp(argv[0], "scalespeed"))
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "Usage: scalespeed {1 | 0}", NULL);
				return TCL_ERROR;
			}
			scaleTempersSpeed = atoi(argv[1]);
			return TCL_OK;
		}
	}
	return Animator::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int ImitatingAnimator::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "speed base offset scalespeed ", NULL);
	return Animator::list(interp);
}
