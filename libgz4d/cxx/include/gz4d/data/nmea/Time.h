// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_TIME_H
#define GZ4D_DATA_NMEA_TIME_H

#include <gz4d/data/nmea/Sentence.h>



namespace gz4d
{
    namespace nmea
    {
        struct Time
        {
            int hour;
            int minute;
            float second;

            Time()
            :hour(-1),minute(-1),second(std::numeric_limits<float>::quiet_NaN())
            {
            }
            
            Time(Sentence const &s, int index)
            {
                if(s.size() > index && s[index].size() >= 6)
                {
                    hour = boost::lexical_cast<int>(s[index].substr(0,2));
                    minute = boost::lexical_cast<int>(s[index].substr(2,2));
                    second = boost::lexical_cast<float>(s[index].substr(4));
                }
            }
            
        };
    }
}

#endif
