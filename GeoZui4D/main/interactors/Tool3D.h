// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Tool3D.h

///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL3D_H
#define TOOL3D_H

#include "interactors/PickInteractor.h"
/** 
 * This is the parent class for the tools that operate in a Scene3D that have
 * the following characteristics:
 *	\li May change the cursor while in that Scene3D
 *	\li May have a hotkey associated with it
 *	\li [future work] May have a GLUI button or menu item associated with it
 *
 * \author Matt Plumlee (mdp)
 * \date 3/3/2000
 */
class Tool3D : public Interactor
{	public:
        Tool3D(std::string name = "");

		//?returnType? getCursor
		int getHotkey();
		virtual const char *getToolName() = 0;
		virtual int select();

		int HandleInput(unsigned int inputType, InputInfo *info);
		virtual void RemovalNotify(InputMediator *fromList);

	protected:
		//derived classes should use preprocessInput at the top of their
		//HandleInput implementatons, unless some behavior is to be overrided
		#define preprocessInput(inputType, info) \
			{   int retval; \
			    retval = Tool3D::HandleInput(inputType, info); \
				if (retval) \
				    return retval; \
			}

		int hotKey;

	private:
		GLboolean disabledPicking;
};

#endif

