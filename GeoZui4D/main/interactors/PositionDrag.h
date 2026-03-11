// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// PositionDrag.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class PositionDrag; //this line connects the above comments with this class.

#ifndef POSITIONDRAG_H
#define POSITIONDRAG_H

#include "engine/Interactor.h"
class PosableStateBase;
class ViewManager;
/**
 *	This class implements a translation interactor that works by enabling the
 *	user to drag the scene around as if it were laying face up on the table.
 *	It does not act as if the user is dragging a world point around, although
 *  it approaches this as the pitch reaches 90 degrees (face up).
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/7/2008
 */
class PositionDrag : public Interactor
{
   public:
		PositionDrag();
		~PositionDrag(void);

		//Interaction functions
		void reset();
		int HandleInput(unsigned int inputType, InputInfo *info);

   protected:
		GLfloat          prevX, prevY;   // Where was the mouse last time?
		PosableStateBase *currentPose;
		ViewManager      *currentView;
};
#endif
