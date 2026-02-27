/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
