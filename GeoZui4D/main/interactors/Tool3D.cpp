/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Tool3D.cpp
// 
// This is the parent class for the tools that operate in a Scene3D that have
// the following characteristics:
//
//		- May change the cursor while in that Scene3D
//		- May have a hotkey associated with it
//		- <future work> May have a GLUI button or menu item associated with it
//
// Created by mdp 3/3/2000
///////////////////////////////////////////////////////////////////////////////

#include "Tool3D.h"
#include "utilities/DottedAddress.h"
#include "interactors/Tool3DPallette.h"

/*---------------------------------------------------------------------------*/
Tool3D::Tool3D(std::string name)
:Interactor(name)
{
	if (!name.empty())
		getAddress()->reparent(globalToolbox.getAddress());
	hotKey = -1; //not a valid key
	disabledPicking = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int Tool3D::getHotkey()
{   return hotKey;
}

/*---------------------------------------------------------------------------*/
int Tool3D::select()
{
	PickInteractor::disable();
	disabledPicking = GL_TRUE;
	return SINK_EVENT;
}

/*---------------------------------------------------------------------------*/
void Tool3D::RemovalNotify(InputMediator *fromList)
{
	if (disabledPicking)
	{
		PickInteractor::enable();
		disabledPicking = GL_FALSE;
	}
}

/*---------------------------------------------------------------------------*/
int Tool3D::HandleInput(unsigned int inputType, InputInfo *info)
{   
	int retval = PASS_EVENT_THROUGH;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
			retval = REMOVE_FROM_STREAM;
	}
	else if (IsCancelAction(inputType, info))
	    retval = REMOVE_FROM_STREAM;
	else if (inputType & KeyStroke && info->val.i == hotKey)
		retval = SINK_EVENT; //block my hotkey!

	return retval;
}
