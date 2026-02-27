/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
