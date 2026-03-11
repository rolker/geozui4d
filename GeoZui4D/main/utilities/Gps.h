// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Gps.h

#ifndef GPS_H
#define GPS_H

#include <stdio.h>
#include "NmeaSource.h"
#include <gz4d/geo/Projection.h>

class TimePieces;

/*---------------------------------------------------------------------------*/
/** 
 *	This RTSource is used to monitor a GPS stream for position, orientation,
 *	speed, and time information.
 *	\note *	Rederived from NmeaSource by mdp 9/21/2004
 *
 *	\author Roland Arsenault (rja)
 *  \date around 9/2/2003
 */
/*---------------------------------------------------------------------------*/
class Gps: public NmeaSource
{
    public:
        void init();
        Gps(std::string a, int s, bool udp_flag = false, fs::path log_filename = "");
        Gps(std::string a, int s, bool udp_flag, NmeaSource *logfileSource);
        Gps(std::string const &fn);
        ~Gps();
    
        double getLon(){return lon;};
        double getLat(){return lat;};
        double getHeading(){return heading;};
        double getCOG(){return cog;};
        double getSOG(){return sog;};
        double getSOGinKnots(){return sog * 1.9438445;};
        double getAltitude(){return altitude;};
        int haveHeading(){return heading_good;};
        int haveCog(){return cog_good;};
        int haveSog(){return sog_good;};
    
        int rmc_time;

    private:
        virtual int process(char * s);
        void decodeTime();
        virtual void resetStates(); //reset things that should be reset at each new file open.
    
        enum GPS_WatchNames {GPS_GGA = 10, GPS_HDT, GPS_VTG, GPS_ZDA, GPS_Date, GPS_RMC, GPS_TT};
    
        double lat, lon;
        double heading,cog,sog,altitude;
        double gps_time;
        int gps_day,gps_month,gps_year, date;
        int gps_date;
        int time_good;
    
        char *temp[4];
        int tempInt;
        double tempDouble;
    
        static TimePieces *tp;
        int heading_good;
        int cog_good, sog_good;
    
        int first_year_seen, last_year_seen;
        static int GGA_flag;
};


#endif
