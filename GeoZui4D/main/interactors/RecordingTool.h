/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// RecordingTool.h

///////////////////////////////////////////////////////////////////////////////

#ifndef RECORDINGTOOL_H
#define RECORDINGTOOL_H

#include "Tool3D.h"
#define RECORDING_HOTKEY '*'

class SessionRecorder;
/** 
 * Interactor for recording user input to a file.
 *
 * \author Matt Plumlee (mdp)
 * \date 8/17/2000
 */
class RecordingTool : public Tool3D
{	public:
		RecordingTool(SessionRecorder *recorder);
		int HandleInput(unsigned int inputType, InputInfo *info);
		const char *getToolName() {return "Record";}
		virtual int select();

	protected:
		virtual int IsCancelAction(int inputType, InputInfo *info);
		SessionRecorder *currentRecorder;

	private:
};

#endif

