// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ViewNudger.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class ViewNudger; //this line connects the above comments with this class.

#ifndef VIEWNUDGER_H
#define VIEWNUDGER_H

#include "engine/ViewManager.h"
#include "engine/Interactor.h"

class PosableStateBase;
class RelationshipQueryResult;
/**
 *	This class implements a translation interactor for effecting small changes
 *	in scen location.
 *	This class implements the following responsibilities:
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/25/2000
 */
class ViewNudger : public Interactor
{
   public:
		//Initialization
		ViewNudger(ViewManager *targetView);
		~ViewNudger(void);

		//Interaction functions
		int HandleInput(unsigned int inputType, InputInfo *info);

		GLboolean startTranslate(InputInfo *info);
		void continueTranslate(void);
		
	protected:
		GLboolean       shiftPressed;          // Is the Shift button currently down?
		GLboolean       cntrlPressed;          // Is the Control key currently down?
		SpecialKey		keyPressed;			   // Which key is pressed?

		ViewManager *currentView;
		RelationshipQueryResult *locksOnWindow; //who do we need to control when changing scale/position?
		PosableStateBase *currentPose;
};
#endif
