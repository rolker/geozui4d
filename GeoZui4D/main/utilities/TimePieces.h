// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// TimePieces.h
///////////////////////////////////////////////////////////////////////////////

#ifndef TIMEPIECES_H
#define TIMEPIECES_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>
#include <SDV_types.h>

/**
 *	This class handles conversions between time represented as a double and
 *	time broken down into month, day, year, etc.
 *  \note Broken out of the TimeControl class 
 *  \author Matt Plumlee (mdp)
 *  \date 9/3/2004
 */
///////////////////////////////////////////////////////////////////////////////
class TimePieces
{
  public:
	TimePieces(double t = std::numeric_limits<double>::quiet_NaN());

	void setYearMonthDay(long yr, int m, int md);
	void setHourMinuteSecond(int hr, int m, double s);
	void getString(char *tstr, float tz, double scale = 1.0);

	void decompose(double t, float tz);
	double compose();

	static int isLeap(long y);
	static int daysInMonth(int month, long y);

	double getSeconds() {return sec;}
	int getMinutes() {return min;}
	int getHour() {return hour;}
	int getDayOfMonth() {return mday;}
	int getMonth() {return mon;}
	long getYear() {return year;}
	int getDayOfWeek() {return wday;}
	int getDayOfYear() {return yday;}
	int getDaysInMonth() {return daysInMonth(mon, year);}

	static const char *getMonthStr(int month) {return months[month];}
	static const char *getDayStr(int weekday) {return dow[weekday];}

  protected:
	double sec;
	int min;
	int hour;
	int mday;
	int mon;
	long year;

	int wday;
	int yday;

	double fullTime;

	// time to string stuff
	static char months[12][4];
	static int days[12];
	static int ldays[12];
	static int tdays[12];
	static int tldays[12];
	static char dow[7][4];

	static int cumulativeDays[400];
};

#endif
