#ifndef GZ4D_DATA_NMEA_ZDA_H
#define GZ4D_DATA_NMEA_ZDA_H

#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/data/nmea/Time.h>
#include <gz4d/time.h>

namespace gz4d
{
    namespace nmea
    {
        struct ZDA: public Time
        {
            int day;
            int month;
            int year;

            ZDA()
            :day(-1),month(-1),year(-1)
            {
            }
            
            ZDA(Sentence const &s)
            :Time(s,1),day(-1),month(-1),year(-1)
            {
                if(!s.IsType("??ZDA"))
                    throw Exception("Invalid type");
                if(s.size() < 6 || s.size() > 7)
                    throw Exception("Wrong size");
                
                day = boost::lexical_cast<int>(s[2]);
                month = boost::lexical_cast<int>(s[3]);
                year = boost::lexical_cast<int>(s[4]);
            }
            
        };
    }
}

#endif
