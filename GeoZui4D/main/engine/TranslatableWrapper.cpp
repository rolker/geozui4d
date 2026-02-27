/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// TranslatableWrapper.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the implementation for wrappers that handle position, but not
 *	necessarily orientation, and scale properties.
 *
 *	Created by mdp 2/21/2002
 */
/*---------------------------------------------------------------------------*/
#include "engine/TranslatableWrapper.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableWrapperSupport.h"
#include "PosableStateUser.h"

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
TranslatableWrapper::TranslatableWrapper(PosableStateUser *initialUser, std::string name)
:PosableStateBase(initialUser, name),
TranslatableState(initialUser, name)
{
}

/*---------------------------------------------------------------------------*/
TranslatableWrapper::TranslatableWrapper(PosableStateBase *toBeReplaced,
									 bool userCall)
:TranslatableState(toBeReplaced)
{
}

/*---------------------------------------------------------------------------*/
TranslatableWrapper::~TranslatableWrapper()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void TranslatableWrapper::replace(PosableStateBase *toBeReplaced)
{
	PosableStateWrapper::replace(toBeReplaced);
}

/*---------------------------------------------------------------------------*/
/**	This is an internal function that checks to see if the wrapped point can
 *	actually be modified before it goes ahead with allowing the modification.
 */
PosableStateInfo *TranslatableWrapper::pointerForMask(long changeMask)
{
	if (askingPointerForSet && !settingLocally && support && !support->indexContentsModifiable(this))
		return PosableStateBase::pointerForMask(changeMask);
	else
	{
		askingPointerForSet = GL_FALSE;
		return TranslatableState::pointerForMask(changeMask);
	}
}
