/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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

