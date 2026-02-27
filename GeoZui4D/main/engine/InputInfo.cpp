/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// InputInfo.cpp
/** 
 * This is a storage class for event information for Interactors.
 *
 * Created by mdp 3/2/2000
 */
///////////////////////////////////////////////////////////////////////////////

#include "engine/InputInfo.h"

long InputInfo::serialNumberGenerator = 0;

/*---------------------------------------------------------------------------*/
InputInfo::InputInfo(ToglWindow *originator, int itype,
		             int addInfo, int vEvent)
: numAdditionalInfo(addInfo),
  additionalInfo((addInfo)? new InputInfo::polyInfo[addInfo] : NULL)
{
	type = itype;
    serialNumber = serialNumberGenerator++;
    virtualEvent = vEvent;
    window = originator;
	eventStream = NULL;
}

/*---------------------------------------------------------------------------*/
InputInfo::InputInfo(int itype, int addInfo, int vEvent)
: numAdditionalInfo(addInfo),
additionalInfo((addInfo)? new InputInfo::polyInfo[addInfo] : NULL)
{
	type = itype;
    serialNumber = serialNumberGenerator++;
    virtualEvent = vEvent;
    window = NULL;
	eventStream = NULL;
}
/*---------------------------------------------------------------------------*/
InputInfo::~InputInfo()
{   if (additionalInfo)
        delete [] additionalInfo;
}
