// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Stopwatch.h

class Stopwatch;

#ifndef STOPWATCH_H
#define STOPWATCH_H
#include <gz4d/time.h>
/*    #ifdef WIN32
    #include <sys/types.h>
    #include <sys/timeb.h>
    #define TIME_STRUCT struct timeb
    #else
    #include <sys/time.h>
    #include <unistd.h>
    #define TIME_STRUCT struct timeval
    #endif*/
/** 
 *	Class for tracking time.  This class has the following responsibilities:
 *	\li Provide a way to start, read, and stop a time measurement.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/21/2000
 */
///////////////////////////////////////////////////////////////////////////////
class Stopwatch
{
	public:
		enum State {NotStarted, Started, Stopped};
		Stopwatch();

		void start();
		double read();
		double sinceLastRead(int update = 1);
		double stop();
		void go();

	protected:
		gz4d::Time startTime, lastReadTime;
		State state;
};

// #ifdef WIN32
// #define DIVISIONS_PER_SECOND 1000
// #else
// #define ftime(arg) gettimeofday(arg, NULL);
// #define time tv_sec
// #define millitm tv_usec
// #define DIVISIONS_PER_SECOND 1000000
// #endif
// 
// #define DIFFERENCE_BETWEEN_TIMES(time1, time2) \
// 	((double)(time2.time - time1.time) + \
// 	(time2.millitm - time1.millitm) / (double)DIVISIONS_PER_SECOND)
// 
// #define SET_TIME(dest_time, src_time) \
// 	dest_time.time = src_time.time; \
// 	dest_time.millitm = src_time.millitm;
// 
#endif
