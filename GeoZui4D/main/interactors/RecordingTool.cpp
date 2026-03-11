// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// RecordingTool.cpp
// 
// Interactor for recording user input to a file.
//
// Created by mdp 8/17/2000
// 
///////////////////////////////////////////////////////////////////////////////

#include "interactors/RecordingTool.h"
#include "engine/SessionRecorder.h"
#include <tcl.h>

/*---------------------------------------------------------------------------*/
RecordingTool::RecordingTool(SessionRecorder *recorder)
:Tool3D("recorder")
{
	hotKey = RECORDING_HOTKEY;
	currentRecorder = recorder;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Set the cancel action to be the same as the recording action.
 */
int RecordingTool::IsCancelAction(int inputType, InputInfo *info)
{
	if (inputType & KeyStroke)
    {   if (info->val.i == hotKey)
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
int RecordingTool::select()
{
	if (currentRecorder->getRecordingState() & SessionRecorder::Playing)
		return REMOVE_FROM_STREAM; //don't record while playing

	if (currentRecorder->record("test.tcl") == TCL_OK)
		return SINK_EVENT;
	else
		return REMOVE_FROM_STREAM;
}

/*---------------------------------------------------------------------------*/
/** This passes any input until the user gives the cancel input.
 */
int RecordingTool::HandleInput(unsigned int inputType, InputInfo *info)
{
	preprocessInput(inputType, info);

	if (info->virtualEvent)
		return PASS_EVENT_THROUGH;

	if ((inputType & KeyStroke) && info->val.i == ' ')
	{
		if (currentRecorder->getRecordingState() == SessionRecorder::Recording)
			currentRecorder->pause();
		else if (currentRecorder->getRecordingState() == SessionRecorder::PausedRecording)
			currentRecorder->record();
	}

	return PASS_EVENT_THROUGH;
}

