// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// HackInteractor.h

///////////////////////////////////////////////////////////////////////////////
class HackInteractor;

#ifndef HACKINTERACTOR_H
#define HACKINTERACTOR_H

#include "engine/Interactor.h"
/**
 * This class has been created as a stop-gap measure until the full scripting
 * interface is available for handling eventStreams and eventHandlers.
 * \li	Call Tcl scripts based on input events
 *
 * \author Matt Plumlee (mdp)
 * \date 3/11/2002
 */
class HackInteractor : public Interactor
{
	public:
		virtual int HandleInput(unsigned int inputType, InputInfo *info);
};
#endif
