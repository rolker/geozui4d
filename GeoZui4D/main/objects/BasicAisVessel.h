/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"BasicVessel.h"


/** This is how our boat represents other vessels in the area that are transmitting AIS data.
 * \par Purpose: 
 * To have a separate class that keeps track of specific AIS data for each vessel that transmits to us.
 * \note Vessel contains the AIS data for our boat.
 * \author Matt Plumlee (mdp)
 * \date 2004
*/

class BasicAisVessel : public BasicVessel
{
    public:
        unsigned int mmsi;
        double lat, lon, lastlat, lastlon;
        float sog, cog, rot;
        double lastContact;
        AIS_Packet danglingInfo;
};




