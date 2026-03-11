// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// TimePieces.h
///////////////////////////////////////////////////////////////////////////////
/**
 *	This class handles conversions between time represented as a double and
 *	time broken down into month, day, year, etc.
 *
 *	Broken out of the TimeControl class by mdp 9/3/2004
 */
///////////////////////////////////////////////////////////////////////////////
#include "utilities/TimePieces.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// #ifdef WIN32
// #include <sys/types.h>
// #include <sys/timeb.h>
// #else
// #include <sys/time.h>
// #include <unistd.h>
// #endif


#include <time.h>
#include <math.h>

#define DAYS_IN_400Y 146097L
#define SECS_IN_DAY 86400L

char TimePieces::months[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
int TimePieces::days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
int TimePieces::ldays[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
int TimePieces::tdays[12] = {0}; //total days in all prior months
int TimePieces::tldays[12] = {0}; //total days in all prior months
char TimePieces::dow[7][4] = {"Thu","Fri","Sat","Sun","Mon","Tue","Wed"};
int TimePieces::cumulativeDays[400] = {0};

//static const double NaN = log(0.0);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include <iostream>

using namespace std;

TimePieces::TimePieces(double t)
{
	fullTime = t;
	
	// setup days arrays if not already done
	if (cumulativeDays[0] == 0)
	{
		int i;

		cumulativeDays[0]=365;
		for (i = 1; i < 400; i++)
		{
			if (isLeap(1970 + i))
				cumulativeDays[i]=cumulativeDays[i-1]+366;
			else
				cumulativeDays[i]=cumulativeDays[i-1]+365;
		}

		for (i = 1; i < 12; i++)
		{
			tdays[i] = tdays[i - 1] + days[i - 1];
			tldays[i] = tldays[i - 1] + ldays[i - 1];
		}
	}

}

/*---------------------------------------------------------------------------*/
void TimePieces::setYearMonthDay(long yr, int m, int md)
{
	mday = md;
	mon = m;
	year = yr;
    fullTime = gz4d::Nan<double>(); //invalidate fullTime
}

/*---------------------------------------------------------------------------*/
void TimePieces::setHourMinuteSecond(int hr, int m, double s)
{
	hour = hr;
	min = m;
	sec = s;
    fullTime = gz4d::Nan<double>(); //invalidate fullTime
}

/*---------------------------------------------------------------------------*/
void TimePieces::decompose(double t, float tz)
{
	int i;
	//double intPart, fPart;

	//Check to see if t even changed since last time...
	if (fullTime == t)
		return;

	//was the intra-second bit the only thing that changed?
/*	if (!isnan(fullTime) && ((int)t == (int)fullTime) && ((fullTime < 0) == (t < 0)))
	{
		fPart = modf(t, &intPart);
		if (t < 0)
			sec = (int)sec + 1 + fPart; //fPart is negative, want to add inverse
		else
			sec = (int)sec + fPart;
		fullTime = t;
		return;
	}*/
	fullTime = t;

	t+=tz*3600.0;

	double day = floor(t / SECS_IN_DAY);
	double dayChunk = day * (double)SECS_IN_DAY; //seconds to start of today

	long s = (long)(t - dayChunk); //get intra-day seconds
	sec =  (t - dayChunk) - s; //this gets the intra-second part

	hour = s / 3600;
	s-= hour * 3600;

	min = s / 60;
	s -= min * 60;

	sec += s;

	wday = (int)(day) % 7;
	if (wday < 0)
		wday += 7;

	double blocks = floor(day / DAYS_IN_400Y);

	day -= blocks * DAYS_IN_400Y;
	double yr = blocks * 400;
	i=0;
	while(day >= cumulativeDays[i])
		i++;

	yr += i;
	yr += 1970;
	year = (long)yr;

	if (i)
		day -= cumulativeDays[i - 1];

	yday = (int)day;

	int month;
	if (isLeap((long)yr))
	{
		for(month = 0; day >= ldays[month]; day -= ldays[month++]){};
	}
	else
	{
		for(month = 0; day >= days[month]; day -= days[month++]){};
	}
	day++;
	mday = (int)day;
	mon = month;
}

/*---------------------------------------------------------------------------*/
void TimePieces::getString(char *tstr, float tz, double scale)
{
	const char *fmt;
	double ascale;

	decompose(fullTime,tz);

	// blank parts of the time display that are moving too fast
	ascale = fabs(scale);
	
	if (ascale < 1.0)
	{
		if (ascale < 0.01)
			fmt = "%s %s %2i %02i:%02i:%09.6f %4i";
		else if (ascale < 0.5) //between 0.01 and 0.5
			fmt = "%s %s %2i %02i:%02i:%06.3f %4i";
		else if (ascale < 1.0) //between 0.5 and 1.0
			fmt = "%s %s %2i %02i:%02i:%04.1f %4i";
		sprintf(tstr, fmt, dow[wday], months[mon], mday, hour, min, sec, year);
	}
	else if (ascale < 100.0) //between 1.0 and 100.0
		sprintf(tstr, "%s %s %2i %02i:%02i:%02i %4li", dow[wday], months[mon], mday, hour, min, (int)sec, year);
	else //(ascale >= 100.0)
	{
		if (ascale >= 10000000.0)
		{
			if (ascale >= 100000000000.0)
				sprintf(tstr,"%lik",year/1000);
			else if (ascale >= 300000000.0) //between 3.0e8 and 1.0e11
				sprintf(tstr,"%4li",year);
			else //between 1.0e7 and 3.0e8
				sprintf(tstr,"%s %4li",months[mon],year);
		}
		else if (ascale >= 2000000.0) //between 1.0e6 and 2.0e7 (added by mdp 9/30/2004)
			sprintf(tstr,"%s %2i %4li",months[mon],mday,year);
		else if (ascale >= 360000.0) //between 3.6e5 and 1.0e6
			sprintf(tstr,"%s %s %2i %4li",dow[wday],months[mon],mday,year);
		else if(ascale >= 6000.0) //between 6,000 and 360,000
			sprintf(tstr,"%s %s %2i %02ih %4li",dow[wday],months[mon],mday,hour,year);
		else //between 100 and 6000
			sprintf(tstr,"%s %s %2i %02i:%02i %4li",dow[wday],months[mon],mday,hour,min,year);
	}
}

/*---------------------------------------------------------------------------*/
double TimePieces::compose()
{
	double daySeconds = sec + (min * 60) + (hour * 3600);
	long block = (long)floor(double(year - 1970) / 400.0);
	int yIndex = (year - 1970) - (block * 400); //index for year including current (need to subtract days in this year off cumulative total)
	yday = (isLeap(year)? tldays[mon] : tdays[mon]) + mday - 1; //(complete days)

	fullTime = ((double)block * DAYS_IN_400Y * SECS_IN_DAY) + (double(cumulativeDays[yIndex] - (isLeap(year)? 366 : 365) + yday) * SECS_IN_DAY) + daySeconds;

	//&&&correct for time zone here

	return fullTime;
}

/*---------------------------------------------------------------------------*/
int TimePieces::isLeap(long y)
{
	if(y%4)
	{
		return 0;
	}
	else if (y%400==0)
	{
		return 1;
	}
	else if (y%100==0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*---------------------------------------------------------------------------*/
int TimePieces::daysInMonth(int month, long y)
{
	if(isLeap(y))
		return ldays[month];
	else
		return days[month];
}

