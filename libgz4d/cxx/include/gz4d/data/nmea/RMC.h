// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_RMC_H
#define GZ4D_DATA_NMEA_RMC_H

#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/data/nmea/Time.h>
#include <gz4d/data/nmea/LatLon.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    namespace nmea
    {
        struct RMC: public Time, public LatLon
        {
            char status;
            float sog; ///< Speed over groud in knots
            float track; ///< Track angle in degrees true
            int day;
            int month;
            int year;
            float magnetic_declination;

            RMC()
            :status('V'),sog(Nan<float>()),track(Nan<float>()),day(-1),month(-1),year(-1),magnetic_declination(Nan<float>())
            {
            }
            
            RMC(Sentence const &s)
            :Time(s,1),LatLon(s,3),status('V'),sog(Nan<float>()),track(Nan<float>()),day(-1),month(-1),year(-1),magnetic_declination(Nan<float>())
            {
                if(!s.IsType("??RMC"))
                    throw Exception("Invalid type");
                if(s.size() < 12 || s.size() > 13)
                    throw Exception("Wrong size");
                
                status = s[2][0];
                if(!s[7].empty())
                    sog = boost::lexical_cast<float>(s[7]);
                if(!s[8].empty())
                    track = boost::lexical_cast<float>(s[8]);
                if(s[9].size() == 6)
                {
                    day = boost::lexical_cast<int>(s[9].substr(0,2));
                    month = boost::lexical_cast<int>(s[9].substr(2,2));
                    year = boost::lexical_cast<int>(s[9].substr(4,2))+2000;
                }
                if(!s[10].empty())
                {
                    magnetic_declination = boost::lexical_cast<float>(s[10]);
                    if (s[11] == "W")
                        magnetic_declination = -magnetic_declination;
                }
                
            }
            
        };
    }
}

#endif
