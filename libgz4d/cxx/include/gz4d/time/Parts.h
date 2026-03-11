// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TIME_PARTS_H
#define GZ4D_TIME_PARTS_H

#include <gz4d/gz4d.h>
//#include <stdint.h>
#include "TimeConstants.h"
#include <cmath>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace gz4d
{
    namespace time
    {
        inline bool isLeap(int64_t y) 
        {
            return !(y%400)|| ((y%100)&&!(y%4));
        }

        struct Parts
        {
            int64_t year;
            uint8_t month;
            uint8_t day;
            uint8_t hour;
            uint8_t minute;
            uint8_t second;
            double frac_second;

            Parts(int64_t year, uint8_t month=1, uint8_t day=1, uint8_t hour=0, uint8_t minute=0, double second=0.0)
            :year(year),month(month),day(day),hour(hour),minute(minute),second(second),frac_second(second-Parts::second)
            {
            }

            Parts(double time)
            {
                boost::posix_time::ptime pt(boost::gregorian::date(1970,1,1),boost::posix_time::seconds(static_cast<long>(time)));
                year = pt.date().year();
                month = pt.date().month();
                day = pt.date().day();
                hour = pt.time_of_day().hours();
                minute = pt.time_of_day().minutes();
                second = pt.time_of_day().seconds();
                frac_second = time-floor(time);
                
//                 int64_t days = floor(time/SECONDS_PER_DAY);
// 
//                 int64_t tyear = (days/DAYS_PER_400Y);
//                 year = 1970 + tyear*400;
//                 days -= tyear*DAYS_PER_400Y;
//                 
//                 tyear = (days/DAYS_PER_NON_LEAP_100Y);
//                 year += tyear*100;
//                 days -= tyear*DAYS_PER_NON_LEAP_100Y;
//                 
//                 tyear = (days/DAYS_PER_4Y);
//                 year += tyear*4;
//                 days -= tyear*DAYS_PER_4Y;
//                 
//                 tyear = (days/DAYS_PER_NON_LEAP_YEAR);
//                 year += tyear;
//                 days -= tyear*DAYS_PER_NON_LEAP_YEAR;
//                 
//                 int cumulative_days[12] = {0,31,59,90,120,151,181,212,243, 273, 304, 334};
//                 for(month = 1; days >= cumulative_days[month] && month < 12; ++month){}
//                 
//                 if(isLeap(year) && month > 2)
//                 {
//                     if(days-cumulative_days[month]==0)
//                         --month;
//                     --days;
//                 }
//                 
//                 days -= cumulative_days[month-1];
//                 
//                 day = days+1;
//                 
//                 int ihour = (int)fmod(floor(time/3600),24);
//                 if (ihour < 0)
//                     hour = ihour += 24;
//                 else
//                     hour = ihour;
// 
//                 int iminute = (int)fmod(floor(time/60),60);
//                 if (iminute < 0)
//                     minute = iminute +60;
//                 else
//                     minute = iminute;
//                 
//                 second = fmod(time,60.0);
//                 if (second < 0.0)
//                     second += 60.0;
            }
            
            uint16_t dayOfYear() const
            {
                int cumulative_days[12] = {0,31,59,90,120,151,181,212,243, 273, 304, 334};
                
                uint16_t ret = cumulative_days[month-1];
                if(month >= 3 && isLeap(year))
                    ret +=1;
                
                ret += day-1;
                return ret;
            }
            
            double secondsSinceEpoch() const
            {
                boost::posix_time::ptime pt(boost::gregorian::date(year,month,day),boost::posix_time::time_duration(hour,minute,second));
                
                boost::posix_time::time_duration ret = pt-boost::posix_time::ptime(boost::gregorian::date(1970,1,1));
                return ret.total_seconds()+frac_second;
                
                
                
                /*int64_t ret =    (year/400)*DAYS_PER_400Y +
                ((year%400)/100)*DAYS_PER_NON_LEAP_100Y +
                ((year%100)/4)*DAYS_PER_4Y +
                (year%4)*DAYS_PER_NON_LEAP_YEAR - DAYS_TO_1970;*/
                
//                 int64_t y = year - 1970;
//                 int64_t ret =    (y/400)*DAYS_PER_400Y +
//                 ((y%400)/100)*DAYS_PER_NON_LEAP_100Y +
//                 ((y%100)/4)*DAYS_PER_4Y +
//                 (y%4)*DAYS_PER_NON_LEAP_YEAR;
//                 ret += dayOfYear();
//                 
//                 return ret * SECONDS_PER_DAY + hour*3600.0 + minute*60.0 + second;
            }
            
            int dayOfWeek() const
            {
                int64_t days = floor(secondsSinceEpoch()/SECONDS_PER_DAY);
                int dow = (days+3)%7;
                if (dow < 0)
                    dow += 7;
                return dow;
            }
            
            std::string monthLabel() const
            {
                char monthLabels[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
                return monthLabels[month-1];
            }
            
            std::string dayLabel() const
            {
                char dayLabels[7][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
                return dayLabels[dayOfWeek()];
            }
        };
    }
}

#endif
