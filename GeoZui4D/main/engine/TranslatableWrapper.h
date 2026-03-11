// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// TranslatableWrapper.h
/*---------------------------------------------------------------------------*/

class TranslatableWrapper;

#ifndef TRANSLATABLE_WRAPPER_H
#define TRANSLATABLE_WRAPPER_H


#include "engine/TranslatableState.h"
#include "engine/PosableStateWrapper.h"
/** 
 *	This is the implementation for wrappers that handle position, but not
 *	necessarily orientation, and scale properties.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 2/21/2002
 */
/*---------------------------------------------------------------------------*/
class TranslatableWrapper : public TranslatableState, public virtual PosableStateWrapper
{
  public:
	/*---Constructors/Destructors---*/
      TranslatableWrapper(PosableStateUser *initialUser = NULL, std::string name = "");
	TranslatableWrapper(PosableStateBase *toBeReplaced, bool userCall = true);
	virtual ~TranslatableWrapper();

  protected:
	virtual void replace(PosableStateBase *toBeReplaced);
	virtual PosableStateInfo *pointerForMask(long changeMask);

};

#endif
