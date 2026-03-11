// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// SessionRecorder.cpp
// 
// Class for recording user input to a file.
//
// Created by mdp 8/18/2000
// 
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include <tcl.h>
#include <string.h>

#include "engine/SessionRecorder.h"
#include "interactors/PlaybackTool.h"
#include "engine/InputMediator.h"
#include <string.h>
extern MainWindow *mainWindow;

#define MAXLINELEN 1024

/*---------------------------------------------------------------------------*/
static SessionRecorder sessionRecorderInstance;
SessionRecorder *sessionRecorder = &sessionRecorderInstance;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
SessionRecorder::SessionRecorder()
{
	int i;

	recordingState = Stopped;
	currentFileName = NULL;
	currentFile = NULL;
	resolution = 25;
	for (i = 0; i < NUM_BOOKMARK_LEVELS; i++)
		bookmarkLevel[i] = 0;

	//int currentBookmarkLevel = 0;

}

/*---------------------------------------------------------------------------*/
SessionRecorder::~SessionRecorder()
{
	if (currentFileName)
		delete currentFileName;
	if (currentFile)
		delete currentFile;
}

/*---------------------------------------------------------------------------*/
int SessionRecorder::RegisterWithTcl(ToglWindow *throughWin)
{
	int retval;

	retval = throughWin->Tcl_LinkVar("recordingState", &recordingState);
	retval |= throughWin->Tcl_LinkVar("recorderResolution", &resolution);
	
	return retval;
}

/*------------------------------Accessors------------------------------------*/
/*---------------------------------------------------------------------------*/
SessionRecorder::RecordingState SessionRecorder::getRecordingState()
{
	return recordingState;
}

/*---------------------------------------------------------------------------*/
const char *SessionRecorder::getSessionFile()
{
	return currentFileName;
}

/*-------------------------------Mutators------------------------------------*/
/*---------------------------------------------------------------------------*/
/** This procedure calls the appropriate function to get the state to the
 *	passed state.  The sessionFile is an optional argument used for Recording
 *	or Playing states.
 */
int SessionRecorder::setState(RecordingState state, const char *sessionFile)
{
	switch (state)
	{
		case Paused:
			return pause();
			break;
		case Recording:
			return record(sessionFile);
			break;
		case Stopped:
			return stop();
			break;
		case Playing:
			return play(sessionFile);
			break;
        default:
            break;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/** Change which file we are using to record or play.  This function stops
 *	all playing or recording to do this.
 */
void SessionRecorder::setSessionFile(const char *sessionFile)
{
	if (recordingState != Stopped)
		stop();
	if (currentFileName)
		delete currentFileName;
	currentFileName = new char[strlen(sessionFile) + 1];
	strcpy(currentFileName, sessionFile);
}

/*---------------------------------------------------------------------------*/
/** This procedure closes the file we are currently recording to or playing
 *	back from.
 */
int SessionRecorder::stop()
{
	if (recordingState == Stopped && currentFile == NULL)
		return TCL_ERROR;

	if (recordingState & Recording)
	{
		mainWindow->Tcl_Eval("endRecord");
		recordTimestamp("SESSION ENDED");
	}

	pausewatch.stop();
	currentFile->flush();
	currentFile->close();
	delete currentFile;
	currentFile = NULL;
	if (currentFileName)
		delete currentFileName;
	currentFileName = NULL;

	recordingState = Stopped;
#ifdef USE_BEEP
	cerr << (char)7; //beep
	cerr << (char)7; //beep
	cerr << (char)7; //beep
#endif
	return TCL_OK;
}

/*-------------------------Recording Procedures------------------------------*/
/*---------------------------------------------------------------------------*/
/** Turn on recording, (open a file for writing).
 */
int SessionRecorder::record(const char *sessionFile)
{
	char pauseComment[64];

	switch (recordingState)
	{
		case PausedRecording:
			recordingState = Recording;
			sprintf(pauseComment, "SESSION PAUSED:%4.6f", pausewatch.stop());
			recordTimestamp(pauseComment);
			stopwatch.go();
			//fallThrough
		case Recording:
			break;
		case PausedPlaying:
		case Playing:
			stop();
			return TCL_ERROR;
			break;
		case Stopped:
			if (sessionFile)
				setSessionFile(sessionFile);
                        currentFile = new std::fstream();
                        currentFile->open(currentFileName, std::ios::out);
			if (!currentFile->good())
			{
				stop();
				return TCL_ERROR;
			}
			recordingState = Recording;
#ifdef USE_BEEP
			cerr << (char)7; //beep
#endif
			stopwatch.start();
			recordTimestamp("SESSION BEGUN");
			mainWindow->Tcl_Eval("beginRecord");
            break;
        default:
            break;
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure stores the passed command to the current session file,
 *	if we are currently recording.
 */
int SessionRecorder::recordCommand(int argc, char *argv[])
{
	char *commandString;

	if (recordingState != Recording)
		return TCL_ERROR;

	commandString = Tcl_Merge(argc, argv);
	recordCommand(commandString);
	Tcl_Free(commandString);

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure stores the passed command to the current session file,
 *	if we are currently recording.
 */
int SessionRecorder::recordCommand(const char *command)
{
	if (recordingState != Recording)
		return TCL_ERROR;

        *currentFile << command << std::endl;
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure stores the passed command to the current session file as
 *	a loop repeated count times, if we are currently recording.
 */
int SessionRecorder::recordCommandBlock(int argc, char *argv[], long count)
{
	char *commandString;

	if (recordingState != Recording)
		return TCL_ERROR;

	commandString = Tcl_Merge(argc, argv);
	recordCommandBlock(commandString, count);
	Tcl_Free(commandString);

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure stores the passed command to the current session file as
 *	a loop repeated count times, if we are currently recording.
 */
int SessionRecorder::recordCommandBlock(const char *command, long count)
{
	char commandString[128];

	if (recordingState != Recording)
		return TCL_ERROR;

	sprintf(commandString, "for {set recLoopCount 0} {$recLoopCount < %ld} {incr recLoopCount} {",
		    count);
	recordCommand(commandString);
	recordCommand(command);
	return recordCommand("}");
}

/*---------------------------------------------------------------------------*/
/** This procedure stores the passed comment to the current session file,
 *	if we are currently recording.
 */
int SessionRecorder::recordTimestamp(const char *comment)
{
	if (recordingState != Recording)
		return TCL_ERROR;

	*currentFile << "#:" << stopwatch.read();
	if (comment)
		*currentFile << " [" << comment << "]";
        *currentFile << std::endl;

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure creates a bookmark in the current session file,
 *	if we are currently recording.  The chapter designation is bumped
 *	up at the passed level (i.e., level 0 is chapter, level 1 is
 *	section, level 2 is subsection, etc.).
 */
int SessionRecorder::recordBookmark(const char *name, int level)
{
	int i;

	if (recordingState != Recording)
		return TCL_ERROR;

	if (level < currentBookmarkLevel)
	{
		for (i = level + 1; i < NUM_BOOKMARK_LEVELS; i++)
			bookmarkLevel[i] = 0;
	}
	if (level >= NUM_BOOKMARK_LEVELS)
		currentBookmarkLevel = NUM_BOOKMARK_LEVELS - 1;
	else
		currentBookmarkLevel = level;
	bookmarkLevel[currentBookmarkLevel]++;

	*currentFile << "#:" << stopwatch.read() << " !{" << bookmarkLevel[0];
	for (i = 1; i <= currentBookmarkLevel; i++)
		*currentFile << "." << bookmarkLevel[i];
	*currentFile << "}";
	if (name && *name)
		*currentFile << " [" << name << "]";
        *currentFile << std::endl;

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure allows other classes to get the time since recording began.
 */
double SessionRecorder::getSessionTime()
{
	return stopwatch.read();
}

/*--------------------------Playback Procedures------------------------------*/
/*---------------------------------------------------------------------------*/
/** This procedure opens a session file of the passed name, but does not
 *	actually play it yet.
 */
int SessionRecorder::selectForPlay(const char *sessionFile)
{
	setSessionFile(sessionFile);
        currentFile = new std::fstream();
        currentFile->open(currentFileName, std::ios::in);
	if (!currentFile->good())
	{
		stop();
		Tcl_AppendResult(mainWindow->interp, "Error opening ", sessionFile, ".", NULL);
		return TCL_ERROR;
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure reads commands from a file and applies them (opens the
 *	session file to do so).  When pausing is implemented, one will be able
 *	to interact as normal upon a pause.
 */
int SessionRecorder::play(const char *sessionFile)
{
	if (recordingState == Stopped && sessionFile)
	{
		if (selectForPlay(sessionFile) != TCL_OK)
			return TCL_ERROR;
	}
	return play((PlaybackTool *)NULL);
}

/*---------------------------------------------------------------------------*/
/** This procedure reads commands from a file and applies them (opens the
 *	session file to do so).  When pausing is implemented, one will be able
 *	to interact as normal upon a pause.
 */
int SessionRecorder::play(PlaybackTool *invokingTool)
{
	int retval;
	GLboolean hadInvokingTool = GL_TRUE;

	switch (recordingState)
	{
		case PausedPlaying:
			recordingState = Playing;
			stopwatch.go();
			//fallThrough
		case Playing:
			break;
		case PausedRecording:
		case Recording:
			stop();
			//fallThrough;
		case Stopped:
			recordingState = Playing;
			if (!invokingTool)
			{
				invokingTool = new PlaybackTool(this);
				globalInputMediator->PushInteractor(invokingTool);
				invokingTool->select();
				hadInvokingTool = GL_FALSE;
			}
			stopwatch.go();
			retval = realPlay();
			if (!hadInvokingTool)
				delete invokingTool;
			return retval;
            break;
        default:
            break;
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure does the actual playing of the session file.
 */
int SessionRecorder::realPlay()
{
	char commandLine[MAXLINELEN];
	double amt;
	float currentTime;
	int t;
	InputInfo info(mainWindow, Interactor::NoInput);

	mainWindow->Tcl_Eval("beginPlayback");
#ifdef USE_BEEP
	cerr << (char)7; //beep
#endif
	while (currentFile && currentFile->good() && (recordingState & Playing))
	{
		if (recordingState & Paused)
		{
			mainWindow->Tcl_Eval("vwait recordingState");
			continue;
		}
		currentFile->getline(commandLine, MAXLINELEN);
		if (*commandLine != '#')
			mainWindow->Tcl_Eval(commandLine);
		else
		{
			if (commandLine[1] == ':') //have a time value
			{
				sscanf(&commandLine[2], "%f", &currentTime);
				sprintf(commandLine, "updatePlaybackTime %g", currentTime);
				mainWindow->Tcl_Eval(commandLine);
			}
			amt = stopwatch.sinceLastRead(GL_FALSE);
			t = int(1000 * amt);
			while (t < resolution) //idle to allow high-res to occur
			{
				mainWindow->Tcl_Eval("global mainWindow; $mainWindow idleCallBack");
				//globalInputMediator->HandleInput(Interactor::NoInput, &info);
				mainWindow->Tcl_Eval("update");
				amt = stopwatch.sinceLastRead(GL_FALSE);
				t = int(1000 * amt);
			}
			stopwatch.sinceLastRead();
		}
	}
	stop();
	mainWindow->Tcl_Eval("endPlayback");
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/** This procedure pauses recording, allowing normal operation
 *	without closing the current file.
 */
int SessionRecorder::pause()
{
	if (recordingState == Recording)
	{
		stopwatch.stop();
		pausewatch.start();
		recordingState = (RecordingState) (recordingState | Paused);
		return TCL_OK;
	}
	return TCL_ERROR;

}
