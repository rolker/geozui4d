// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// TranslatableState.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for classes that have position, but not
 *	necessarily orientation, and scale properties.
 *
 *	Created by mdp 12/13/2000
 */
/*---------------------------------------------------------------------------*/
#include "engine/TranslatableState.h"
#include "widgets/Sv_Standards.h"
#include "PosableStateUser.h"
#include "engine/ViewManager.h"
#include <gz4d/glo/Translate.h>

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
TranslatableState::TranslatableState(PosableStateUser *initialUser, std::string name)
:PosableStateBase(initialUser, name)
{
  _position[X] = 0.0;
  _position[Y] = 0.0;
  _position[Z] = 0.0;

  _homePosition[X] = 0.0;
  _homePosition[Y] = 0.0;
  _homePosition[Z] = 0.0;
}

/*---------------------------------------------------------------------------*/
TranslatableState::TranslatableState(PosableStateBase *toBeReplaced,
									 GLboolean userCall)
:PosableStateBase(toBeReplaced)
{
	_position[X] = 0.0;
	_position[Y] = 0.0;
	_position[Z] = 0.0;

	_homePosition[X] = 0.0;
	_homePosition[Y] = 0.0;
	_homePosition[Z] = 0.0;

	if (userCall)
		replace(toBeReplaced);
}

/*---------------------------------------------------------------------------*/
TranslatableState::~TranslatableState()
{
}

/*-----------------------------------Accessors-------------------------------*/
/**	Even though we don't really have a PosableStateInfo, we'll fake it.
 */
PosableStateInfo *TranslatableState::pointerForMask(long changeMask)
{
	if (changeMask & (ChangeRotateMask | ChangeScaleMask))
		return PosableStateBase::pointerForMask(changeMask);
	if (changeMask & ChangeCurrentMask)
		return (PosableStateInfo*)&_position.front();
	else if (changeMask & ChangeHomeMask)
		return (PosableStateInfo*)_homePosition;
	else
		return PosableStateBase::pointerForMask(changeMask);
}

/*---------------------------------------------------------------------------*/
/**	supportMask returns the portion of the mask that is supported by this
 *	posable.  Returns ChangeNone if nothing of the type passed is supported.
 */
int TranslatableState::supportMask(int changeMask)
{
	if (changeMask & ChangeCurrentMask)
		return (changeMask & ChangeTranslate);
	else if (changeMask & ChangeHomeMask)
		return (changeMask & ChangeHomeTranslate);
	else
		return ChangeNone;
}

/*-----------------------------GL Transformations----------------------------*/
/*---------------------------------------------------------------------------*/
void TranslatableState::applyTransforms(ViewManager * view)
{
    if(view)
        gz4d::glo::Translate(_position-view->getReferencePoint());
    else
        gz4d::glo::Translate(_position);
}

