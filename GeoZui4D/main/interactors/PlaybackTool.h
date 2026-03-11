// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// PlaybackTool.h

///////////////////////////////////////////////////////////////////////////////

#ifndef PLAYBACKTOOL_H
#define PLAYBACKTOOL_H

#include "Tool3D.h"

class SessionRecorder;
/** 
 * Interactor for playing back user input from a file.
 *
 * \author Matt Plumlee (mdp)
 * \date 8/18/2000
 */
class PlaybackTool : public Tool3D
{	public:
		PlaybackTool(SessionRecorder *recorder);
		int HandleInput(unsigned int inputType, InputInfo *info);
		const char *getToolName() {return "Playback";}
		virtual int select();

	protected:
		virtual int IsCancelAction(int inputType, InputInfo *info);
		SessionRecorder *currentRecorder;
		int startedPlaying;

	private:
};

#endif

