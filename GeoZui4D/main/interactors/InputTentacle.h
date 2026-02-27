/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// InputTentacle.h

///////////////////////////////////////////////////////////////////////////////

#ifndef INPUTTENTACLE_H
#define INPUTTENTACLE_H

#include "interactors/PickInteractor.h"
/** 
 *	This class acts as a projection of another interactor for intercepting
 *	particular types of events (from a different place in the input stream
 *	than the owner of the tentacle).
 *
 * \author Matt Plumlee (mdp)
 * \date 2/5/2003
 */
class InputTentacle : public Interactor
{	public:
        InputTentacle(std::string name = "");

		void setOwner(Interactor *newOwner);
		void setFilter(unsigned int inputType, GLboolean sendNoInput = GL_FALSE);
		void ignoreBefore(long serialNumber);
		long getIgnoreBefore() {return ignoreBeforeSN;}

		int HandleInput(unsigned int inputType, InputInfo *info);

		/*---Scripting funcitons---*/
		virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
		virtual int list(Tcl_Interp *interp);

	protected:
		Interactor *owner;
		unsigned int inputFilter;
		GLboolean passAlongNoInput;
		long ignoreBeforeSN;
};

#endif

