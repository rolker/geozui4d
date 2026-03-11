// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Tool3DPallette.cpp
// 
// This class handles selection of Tool3D's.
//
// Created by mdp 3/3/2000
///////////////////////////////////////////////////////////////////////////////

#include "interactors/Tool3DPallette.h"
#include "engine/InputMediator.h"
#include "Tool3D.h"
#include "engine/ToglWindow.h"

#define MAX_KEYS 128
#define MAX_TOOL_COMMAND 256

Tool3DPallette globalToolbox(globalInputMediator);

/*---------------------------------------------------------------------------*/
Tool3DPallette::Tool3DPallette(InputMediator *targetOfTool)
:Interactor("toolPallette")
{
	int i;

    toolTarget = targetOfTool;
    hotKeyArray = new Tool3D*[MAX_KEYS];
	for (i = 0; i < MAX_KEYS; i++)
	    hotKeyArray[i] = NULL;

	enabled = GL_TRUE;
	windowForRegistering = NULL;
}

/*---------------------------------------------------------------------------*/
int Tool3DPallette::RegisterWithTcl(ToglWindow *throughWin)
{
	int i;
	Tool3D *currentTool;
	char tclCommand[MAX_TOOL_COMMAND];

	windowForRegistering = throughWin;

	//---Add existing tools to the tools menu---
	for (i = 0; i < MAX_KEYS; i++)
	{
		currentTool = hotKeyArray[i];
		if (!currentTool)
			continue;

		sprintf(tclCommand, "addTool \"%s\" %c", currentTool->getToolName(), currentTool->getHotkey());
		throughWin->Tcl_Eval(tclCommand);
	}

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
Tool3DPallette::~Tool3DPallette()
{
	delete [] hotKeyArray;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int Tool3DPallette::HandleInput(unsigned int inputType, InputInfo *info)
{
	Tool3D *currentTool;
	int retval;

	if (!enabled || (inputType & EnumeratedAction))
		return PASS_EVENT_THROUGH;

    if (inputType & KeyStroke && info->val.i < MAX_KEYS)
    {   currentTool = hotKeyArray[info->val.i];
	    if (currentTool)
		{
			retval = currentTool->select();
			if (retval == REMOVE_FROM_STREAM)
			{
				return SINK_EVENT;
			}
			toolTarget->PushInteractor(currentTool);
		    //Would set cursor here too?
			return retval;
		}
	}
	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/* Try to add the tool.  If the hotkey is already taken, then fail.
 */
int Tool3DPallette::addTool(Tool3D *newTool)
{
	char tclCommand[MAX_TOOL_COMMAND];
	int newHotkey;
	
	if (newTool)
	{
		newHotkey = newTool->getHotkey();

		if (hotKeyArray[newHotkey])
			return TCL_ERROR;

		hotKeyArray[newHotkey] = newTool;
		sprintf(tclCommand, "addTool \"%s\" %c", newTool->getToolName(), newHotkey);
	}
	else
		sprintf(tclCommand, "addTool");

	if (windowForRegistering)
		windowForRegistering->Tcl_Eval(tclCommand);

	return TCL_OK;
}
