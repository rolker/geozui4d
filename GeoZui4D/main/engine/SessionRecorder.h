// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// SessionRecorder.h

///////////////////////////////////////////////////////////////////////////////
class SessionRecorder;

#ifndef SESSIONRECORDER_H
#define SESSIONRECORDER_H

#include <fstream>
#include "tclLinkage.h"
#include "utilities/Stopwatch.h"


#define NUM_BOOKMARK_LEVELS 10

struct Tcl_Interp;
class ToglWindow;
class PlaybackTool;
/** 
 *  Class for recording user input to a file.  This class has the following
 *  responsibilities:
 *  \li Allow for recording a session to a file as well as not recording
 *  \li Provide a way to play back a recorded session.
 *
 *  \author Matt Plumlee (mdp)
 *  \date 8/18/2000
 */
class SessionRecorder
{   public:
        enum RecordingState {Stopped = RECORDER_STOPPED,
                             Recording = RECORDER_RECORDING,
                             Playing = RECORDER_PLAYING,
                             Paused = RECORDER_PAUSED,
                             PausedRecording = RECORDER_PAUSED_RECORDING,
                             PausedPlaying = RECORDER_PAUSED_PLAYING};
        SessionRecorder();
        int RegisterWithTcl(ToglWindow *throughWin);
        ~SessionRecorder();

        /*---Accessors---*/
        RecordingState getRecordingState();
        const char *getSessionFile();
        double getSessionTime();

        /*---Actions---*/
        void setSessionFile(const char *sessionFile);
        int record(const char *sessionFile = NULL);
        int recordCommand(int argc, char *argv[]);
        int recordCommand(const char *command);
        int recordCommandBlock(int argc, char *argv[], long count);
        int recordCommandBlock(const char *command, long count);
        int recordTimestamp(const char *comment = NULL);
        int recordBookmark(const char *name = NULL, int level = 0);
        int pause();
        int stop();
        int play(const char *sessionFile = NULL);
        int play(PlaybackTool *invokingTool);
        int selectForPlay(const char *sessionFile);
        int setState(RecordingState state, const char *sessionFile = NULL);

    protected:
        Stopwatch stopwatch, pausewatch;
        RecordingState recordingState;
        char *currentFileName;
        std::fstream *currentFile;
        int resolution;
        int bookmarkLevel[NUM_BOOKMARK_LEVELS];
        int currentBookmarkLevel;

        int realPlay();
};

extern SessionRecorder *sessionRecorder;

#endif

