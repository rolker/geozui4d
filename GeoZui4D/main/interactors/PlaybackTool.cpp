// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// PlaybackTool.cpp
// 
// Interactor for playing back user input from a file.
//
// Created by mdp 8/18/2000
// 
///////////////////////////////////////////////////////////////////////////////

#include "interactors/PlaybackTool.h"
#include "interactors/RecordingTool.h"
#include "engine/SessionRecorder.h"
#include "engine/ToglWindow.h"
#include <tcl.h>

#define MAXCOMMAND 128
/*---------------------------------------------------------------------------*/
PlaybackTool::PlaybackTool(SessionRecorder *recorder)
:Tool3D("playback")
{
	hotKey = '>';
	currentRecorder = recorder;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Set the cancel action to be the same as the playback or recording action.
 */
int PlaybackTool::IsCancelAction(int inputType, InputInfo *info)
{
	if (inputType & KeyStroke)
    {   if (info->val.i == hotKey || info->val.i == RECORDING_HOTKEY)
		{
			currentRecorder->stop();
	        return 1;
		}
	}
	return 0;
}

/*---------------------------------------------------------------------------*/
/** This causes the recorder to start recording when the Recroding tool is
 *	selected.
 */
int PlaybackTool::select()
{
	if (currentRecorder->getRecordingState() == SessionRecorder::Stopped)
	{
		startedPlaying = GL_FALSE;
		if (currentRecorder->selectForPlay("demo.tcl") == TCL_OK)
			return SINK_EVENT;
		else
			return REMOVE_FROM_STREAM;
	}
	else
	{
		startedPlaying = GL_TRUE;
		return SINK_EVENT;
	}
}

/*---------------------------------------------------------------------------*/
/** This passes any input until the user gives the cancel input.
 */
int PlaybackTool::HandleInput(unsigned int inputType, InputInfo *info)
{
	//preprocessInput(inputType, info); //we don't want our input preprocessed.

	if (!startedPlaying)
	{	//The play function wrests control, and is called only once
		//before this tool is removed.  However, it causes the HandleInput
		//function to be called repeatedly before it returns.
		startedPlaying = GL_TRUE;
		currentRecorder->play(this);
		globalInputMediator->Trigger(BreakAction);
		return REMOVE_FROM_STREAM;
	}

	if (info->virtualEvent)
		return PASS_EVENT_THROUGH;

	if (inputType & KeyStroke)
	{
		if (info->val.i == RECORDING_HOTKEY || info->val.i == hotKey)
			return SINK_EVENT;
	}

	if (inputType & MouseAction)
	{
		info->window->MoveMouse(info->x.i, info->y.i);
	}

	return PASS_EVENT_THROUGH;
}

