// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Gps.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This RTSource is used to monitor a GPS stream for position, orientation,
 *	speed, and time information.
 *	
 *
 *	Created by roland around 9/2/2003
 *	Rederived from NmeaSource by mdp 9/21/2004
 */
/*---------------------------------------------------------------------------*/
#include "utilities/Gps.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <filesystem>
//#include <time.h>
#include "utilities/TimePieces.h"
#include "engine/Scene3D.h"
#include "MainWindow.h"

using namespace std;

extern MainWindow * mainWindow;

TimePieces *Gps::tp = new TimePieces;
int Gps::GGA_flag = 0;
/*---------------------------------------------------------------------------*/
void Gps::init()
{
    static NMEA_ValueType GGA_types[] = {Double,    TempString, TempString, TempString, TempString, Integer,  Skip, Skip, Double};
    static void *GGA_vars[] = {&gps_time, &temp[0],  &temp[1],   &temp[2],   &temp[3],   &tempInt,  NULL, NULL, &altitude};

    static NMEA_ValueType RMC_types[] = {Double,  Skip, TempString, TempString, TempString, TempString, Double, Double};
    static void *RMC_vars[] = {&gps_time, NULL, &temp[0],  &temp[1],   &temp[2],   &temp[3],   &sog, &cog};

    static NMEA_ValueType VTG_types[] = {Double, TempString, Skip, Skip, Double, Double};
    static void *VTG_vars[] = {&cog,  &temp[0],   NULL, NULL, &sog,   &tempDouble};

    static NMEA_ValueType ZDA_types[] = {Double,     Integer,  Integer,    Integer};
    static void *ZDA_vars[] = {&gps_time, &gps_day, &gps_month, &gps_year};
        
    static NMEA_ValueType TT_types[] = {TempString, Integer, Integer};
    static void *TT_vars[] = {&temp[0], &gps_date, &gps_time};

    shouldLogGzTime = GL_FALSE;
    resetStates(); //although already called by parent constructor, it was before our virtual table was updated to get the call right.

    watchFor('$', "GGA", GPS_GGA, 9, GGA_types, GGA_vars);
    watchFor('$', "RMC", GPS_RMC, 8, RMC_types, RMC_vars);
    watchFor('$', "HDT", GPS_HDT, 0, Double, &heading);
    watchFor('$', "VTG", GPS_VTG, 6, VTG_types, VTG_vars);
    watchFor('$', "ZDA", GPS_ZDA, 4, ZDA_types, ZDA_vars);
    watchFor('$', "TT", GPS_TT, 3, TT_types, TT_vars); //$PSTT,10A,20080731,175158*6F
}

/*---------------------------------------------------------------------------*/
void Gps::resetStates()
{
    lat=lon=heading=cog=sog=altitude=0.0;
    time_good=0;
    heading_good = 0;
    cog_good = 0;
    sog_good = 0;
    first_year_seen = -10000;
    rmc_time = 0;

    GGA_flag = 0;

    NmeaSource::resetStates();
}

/*---------------------------------------------------------------------------*/
Gps::Gps(string a, int s, bool udp_flag, fs::path log_filename)
    : NmeaSource(a, s, udp_flag, log_filename)
{
    init();
}

/*---------------------------------------------------------------------------*/
Gps::Gps(string a, int s, bool udp_flag, NmeaSource *logfileSource)
    : NmeaSource(a, s, udp_flag, logfileSource)
{
    init();
}

/*---------------------------------------------------------------------------*/
Gps::Gps(std::string const &fn)
    : NmeaSource(fn)
{
    init();
}

/*---------------------------------------------------------------------------*/
Gps::~Gps()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int Gps::process(char * s)
{
    char nf,ef;
    int message;
	//cerr << s << endl;
	
    message = NmeaSource::process(s);
	
	
	/** To make sure GGA data takes precidence over RMC data.
     * Used a static variable in the gps class (GGA_flag) to make sure that GGA data 
     *	is used instead of RMC data (if applicable). 
     * \author briana sullivan (bs)
     * \date 6/28/05
     *	\note If RMC is found after GGA data is flagged then it will be skipped 
     *	(message = NOT_PROCESSED) 
         */	
    if(message == GPS_GGA)
        GGA_flag = 1;
    else if(GGA_flag && (message == GPS_RMC))
        message = NOT_PROCESSED;

    switch (message)
    {
        case NOT_PROCESSED:
            break;
        case GPS_GGA:
        {
            //message = GPS_GGA
            if (fieldsFound < 2)
                break;

            if (time_good)
                decodeTime();
            lat = floor(atof(temp[0]) / 100);
            double n_min = atof(&temp[0][2]);
            lat += n_min/60.0;
            if (fieldsFound < 3)
                break;

            nf = temp[1][0];
            if (fieldsFound < 4)
                break;

            lon = floor(atof(temp[2]) / 100);
            int e_min = atof(&temp[2][3]);
            lon += e_min/60.0;
            if (fieldsFound < 5)
                break;

            ef = temp[3][0];
            if (fieldsFound < 6)
                break;

            if(tempInt)
            {
                if(nf=='S')
                    lat=-lat;
                if(ef=='W')
                    lon=-lon;
            }              
            else
            {
                cerr << "received invalid GPS fix\n";
                message = NOT_PROCESSED;
            }
        }
            break;

        case GPS_RMC:
        {
			//message = GPS_RMC;
            if (fieldsFound < 2)
                break;

            previousTime = currentTime;
            if (rmc_time)
                currentTime++;
            else if (gps_time == 0)
            {
                rmc_time = 1;
                currentTime = lastTimeRead;
            }
            if (fieldsFound < 3)
                break;

            lat = floor(atof(temp[0]) / 100);
            double n_min = atof(&temp[0][2]);
            lat += n_min/60.0;
            if (fieldsFound < 4)
                break;

            nf = temp[1][0];
            if(nf=='S')
                lat=-lat;
            if (fieldsFound < 5)
                break;

            lon = floor(atof(temp[2]) / 100);
            double e_min = atof(&temp[2][3]);
            lon += e_min/60.0;
            if (fieldsFound < 6)
                break;

            ef = temp[3][0];
            if(ef=='W')
                lon=-lon;
            if (fieldsFound < 8)
                break;

            cog_good=1;
            sog = sog * 0.5144444f; //knots to meters per second
            sog_good=1;
        }
            break;

        case GPS_HDT:
			//message = GPS_HDT;
            if (fieldsFound)
                heading_good = 1;
            break;

        case GPS_VTG:
			//message = GPS_VTG;
            if (fieldsFound < 1)
                break;

            cog_good=1;;
            if (fieldsFound < 5)
                break;

            if (rmc_time)
                break; //added by mdp 6/20/2005 so RMC sog would supercede VTG sog.

			/* //mdp 7/29/2005... not sure why this was here...
            if (temp[0][0] != 'M')  //mag cog
            {
            if (fieldsFound < 6)
            return GPS_VTG;
            sog = tempDouble;
    }
                        */
			//sog = sog/3.6; //km/h->m/s
            sog = sog * 0.5144444f; //knots to meters per second
            sog_good=1;
            break;

        case GPS_ZDA:
			//message = GPS_ZDA;
            if (fieldsFound >= 4)
            {
                time_good = 1;
                decodeTime();
            }
            break;
        case GPS_TT:
            if (fieldsFound >= 3)
            {
                std::string t = temp[0];
                if (t == "10A")
                {
                    gps_year = static_cast<int>(gps_date/10000);
                    gps_month = static_cast<int>((gps_date%10000)/100);
                    gps_day = static_cast<int>(gps_date%100);
                    time_good = 1;
                    decodeTime();
                    break;
                }
            }
            message = NOT_PROCESSED;
            break;
        default:
            break;
    }

    if(s[0]=='$')
    {
        if(isType(s,"te,"))
        {
            sscanf(s,"$Date,%i-%i-%i",&gps_month,&gps_day,&gps_year);
            time_good = 1;
            decodeTime();
            return GPS_Date;
        }
    }
	//NmeaSource::printString(s);
    return message;
}

/*---------------------------------------------------------------------------*/
void Gps::decodeTime()
{
    int hours,minutes;
    double seconds;

    hours = (int)(gps_time/10000);
    minutes = (int)((gps_time-(hours*10000))/100);
    seconds = gps_time-(hours*10000)-(minutes*100);

	//something of a hack to make it less likely that a corrupt gps year
	//will throw us off.
    if (first_year_seen <= -10000)
        last_year_seen = first_year_seen = gps_year;
    else
    {
        if (gps_year > first_year_seen)
        {
            if (gps_year > last_year_seen + 1)
                return;
            if (gps_year > last_year_seen)
                last_year_seen = gps_year;
        }
    }
    tp->setYearMonthDay(gps_year, gps_month - 1, gps_day);
    tp->setHourMinuteSecond(hours, minutes, seconds);
	
    seconds = tp->compose();
    if (glossOverHiccups && (seconds >= previousTime))
    {
        previousTime = currentTime;
        currentTime = seconds;
    }
  //cerr << "Seconds: " << currentTime << endl;
}

