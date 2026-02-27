/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef RTSOURCE
#define RTSOURCE

/**
 *  This class represents the real time source coming from a port that is hooked up to a sensor.
 *  A file that is not preloaded is also considered a "real time" source but only while it is loading,
 *  as soon as it is done loading it is equivalent to the sensor being disconnected from a port.
 *  \note Any replay of the data loaded is considered playback not real-time.
 *
 *  \author Roland Arsenault (rja)
 *  \date 2003
 */
class RTSource
{
public:
	RTSource();
	virtual ~RTSource();
	double GetCurrentTime(){return currentTime;};

	int glossOverHiccups; //glossOverHiccups and previousTime added by mdp 7/15/2004

protected:
	double currentTime, previousTime;
};
#endif
