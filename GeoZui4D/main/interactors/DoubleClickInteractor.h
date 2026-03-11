// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// DoubleClickInteractor.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class DoubleClickInteractor; //this line connects the above comments with this class.

#ifndef DOUBLECLICKINTERACTOR_H
#define DOUBLECLICKINTERACTOR_H

#include "engine/Interactor.h"

#define CLICK_EVENT DoubleClickInteractor::clickEvent()
#define DOUBLE_CLICK_EVENT DoubleClickInteractor::doubleClickEvent()
/**
 *	This class implements a double-click interactor for properly introducing
 *	double-clicks to this environment.
 *	This class implements the following responsibilities:
 *	\li	Provide mechanisms for customizing the sloppiness of double-clicks.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 9/8/2000
 */
class DoubleClickInteractor : public Interactor
{
   public:
		//Initialization
		DoubleClickInteractor();
		~DoubleClickInteractor(void);

		//Interaction functions
		int HandleInput(unsigned int inputType, InputInfo *info);
		static unsigned int clickEvent() { return clickInputType->value;}
		static unsigned int doubleClickEvent() { return doubleClickInputType->value;}
		virtual int test(InputInfo *info);
		void reset();

		//Tweaking
		void setMaxMovement(int pixels);
		void setMaxIdle(int idleCycles);

	protected:
		int clickX, clickY;
		int doubleClickX, doubleClickY;
		int maxMove, maxIdle;
		int idleCount;
		unsigned int clickButton, doubleClickButton;

		static const EventType *clickInputType;
		static const EventType *doubleClickInputType;
};

extern DoubleClickInteractor *doubleClickInteractor;
#endif
