// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// Stopwatch.cpp
// 
//	Class for tracking time.
//
// Created by mdp 8/21/2000
//
///////////////////////////////////////////////////////////////////////////////

#include "utilities/Stopwatch.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Constructor.
 */
Stopwatch::Stopwatch()
{
	state = NotStarted;
}

/*----------------------------Stopwatch Interface----------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	This procedure starts the stopwatch, setting the lastReadTime to
 *	the same value as startTime.
 */
void Stopwatch::start()
{
    startTime = gz4d::time::Now();
    lastReadTime = startTime;
    state = Started;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure reads the stopwatch, returning the number of seconds since
 *	the stopwatch was started or the time elapsed when it was stopped.
 *	Returns -1.0 if the	stopwatch was never started.
 */
double Stopwatch::read()
{
	if (state == NotStarted)
		return -1.0;

	if (state == Started)
            lastReadTime = gz4d::time::Now();

	return (lastReadTime-startTime).asSeconds();
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure returns the time passed since the last time read() was called
 *	or sinceLastRead() was called and GL_FALSE is not passed, if the stopwatch
 *  is going.  If it is stopped, sinceLastRead returns the time elapsed since
 *	the stopwatch was stopped.  Returns -1.0 if the stopwatch was never started.
 */
double Stopwatch::sinceLastRead(int update)
{
	//TIME_STRUCT currentTime;
	double retval;

	if (state == NotStarted)
		return -1.0;

	if (state == Stopped)
		return read();

	//ftime(&currentTime);
        gz4d::Time currentTime = gz4d::time::Now();

    retval = (currentTime-lastReadTime).asSeconds();
	if (update)
	{
            lastReadTime = currentTime;
	}

	return retval;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure stops the stopwatch, recording the time of stopping for
 *	subsequent read and sinceLastRead operations.
 */
double Stopwatch::stop()
{
	double retval = read();

	state = Stopped;

	return retval;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure allows the stopwatch to continue after it has been stopped.
 *	I would have used the word continue if it weren't a reserved word.
 *	(Since it is go, however, then it will act the same as start if the
 *	stopwatch has never been started)
 */
void Stopwatch::go()
{
	double elapsedTime;

	if (state == NotStarted)
	{
		start();
		return;
	}
	else if (state == Started)
		return;

	//we do it the following way so we don't have to fiddle with the
	//cases where the milliseconds are negative.
	state = Started;
	elapsedTime = sinceLastRead(); //fool it into taking the time since we stopped it.
	startTime += gz4d::seconds(elapsedTime);

}
