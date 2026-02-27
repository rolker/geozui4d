/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/


#include "engine/RTSource.h"

RTSource::RTSource()
{
	previousTime = currentTime = 0.0;
	glossOverHiccups = 1;
}

RTSource::~RTSource()
{
}

