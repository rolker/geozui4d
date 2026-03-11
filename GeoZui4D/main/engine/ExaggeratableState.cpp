// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// ExaggeratableState.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, but not
 *	necessarily orientation, and scale properties.
 *
 *	Created by mdp 1/10/2002
 */
/*---------------------------------------------------------------------------*/
#include "ExaggeratableState.h"
#include "widgets/Sv_Standards.h"
#include "PosableStateUser.h"

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
ExaggeratableState::ExaggeratableState(PosableStateUser *initialUser, const char *name)
:TranslatableState(initialUser, name)
{
	_scale = _homeScale = 1.0;
}

/*---------------------------------------------------------------------------*/
ExaggeratableState::ExaggeratableState(PosableStateBase *toBeReplaced,
									   GLboolean userCall)
:TranslatableState(toBeReplaced, GL_FALSE)
{
	_scale = _homeScale = 1.0;
	if (userCall)
		replace(toBeReplaced);
}

/*---------------------------------------------------------------------------*/
ExaggeratableState::~ExaggeratableState()
{
}

/*-----------------------------------Accessors-------------------------------*/
/*---------------------------------------------------------------------------*/
/**	getPoseInfo returns position, orientation, and scale information back
 *	through the passed arguments.  If a class does not implement one of
 *	these attributes, it should not change the passed values.
 */
void ExaggeratableState::getPoseInfo(gz4d::Point<double> &position, OrientationInfo &rot, double &scale, int changeMask)
{
	if (changeMask & ChangeScaleMask)
	{
		if (changeMask & ChangeCurrentMask)
			scale = _scale;
		else if (changeMask & ChangeHomeMask)
			scale = _homeScale;
	}

	TranslatableState::getPoseInfo(position, rot, scale, changeMask);
}

/*---------------------------------------------------------------------------*/
/**	supportMask returns the portion of the mask that is supported by this
 *	posable.  Returns ChangeNone if nothing of the type passed is supported.
 */
int ExaggeratableState::supportMask(int changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return (changeMask & (ChangeTranslate | ChangeScale));
	else if (changeMask & ChangeHomeMask)
		return (changeMask & (ChangeHomeTranslate | ChangeHomeScale));
	else
		return ChangeNone;
}

/*---------------------------------------------------------------------------*/
/**	getScale returns a scalar scale value, if supported by this class, otherwise
 *	returns NULL.  This can be used to see if a particular orientation
 *	attribute is presently set.
 */
double ExaggeratableState::getScale(int changeMask)
{
	if (changeMask & ChangeScaleMask)
	{
		if (changeMask & ChangeCurrentMask)
			return _scale;
		if (changeMask & ChangeHomeMask)
			return _homeScale;
	}
	return PosableStateBase::getScale(changeMask);
}

/*------------------------------------Mutators-------------------------------*/
/*---------------------------------------------------------------------------*/
void ExaggeratableState::setScale(double sval, int changeMask)
{
	long whatChanged = changeMask & ChangeTypeMask;
	float *scale_ptr;

	if (!(changeMask & ChangeScaleMask))
		return;

	if (changeMask & ChangeCurrentMask)
		scale_ptr = &_scale;
	else if (changeMask & ChangeHomeMask)
		scale_ptr = &_homeScale;
	else
		return;

	if (sval != *scale_ptr)
	{
		whatChanged |= ChangeScaleMask;
		*scale_ptr = sval;
	}

	if (whatChanged)
		NotifyPose(whatChanged);
}

/*---------------------------------------------------------------------------*/
void ExaggeratableState::scaleBy(double sval, int changeMask)
{
	if (!(changeMask & ChangeScaleMask))
		return;

	if (changeMask & ChangeCurrentMask)
		_scale *= sval;
	else if (changeMask & ChangeHomeMask)
		_homeScale *= sval;
	else
		return;

	NotifyPose((changeMask & ChangeScaleMask) |
			   (changeMask & ChangeTypeMask));
}

/*-----------------------------GL Transformations----------------------------*/
/*---------------------------------------------------------------------------*/
void ExaggeratableState::applyTransforms(ViewManager * view)
{
	glScaled(1.0, 1.0, _scale);
	TranslatableState::applyTransforms(view);
}

