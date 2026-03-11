// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_GGA_H
#define GZ4D_DATA_NMEA_GGA_H

#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/data/nmea/Time.h>
#include <gz4d/data/nmea/LatLon.h>

namespace gz4d
{
    namespace nmea
    {
        struct GGA: public Time, public LatLon
        {
            enum FixQuality
            {
                Invalid = 0, 
                GPS = 1,
                DGPS = 2,
                PPS = 3,
                RTK = 4,
                Float_RTK = 5,
                Dead_Reckoning = 6,
                Manual,
                Simulation
            };
            int quality;
            int sat_count;
            float hdop;
            float altitude; ///< Altitude above the geoid
            float geoid_height; ///< Height of the geoid relative to the ellipsoid
            float dgps_age;
            int dgps_id;

            GGA()
            :quality(Invalid),sat_count(-1),hdop(std::numeric_limits<float>::quiet_NaN()),altitude(std::numeric_limits<float>::quiet_NaN()),geoid_height(std::numeric_limits<float>::quiet_NaN()),dgps_age(std::numeric_limits<float>::quiet_NaN()),dgps_id(-1)
            {
            }
            
            GGA(Sentence const &s)
            :Time(s,1),LatLon(s,2),quality(Invalid),sat_count(-1),hdop(std::numeric_limits<float>::quiet_NaN()),altitude(std::numeric_limits<float>::quiet_NaN()),geoid_height(std::numeric_limits<float>::quiet_NaN()),dgps_age(std::numeric_limits<float>::quiet_NaN()),dgps_id(-1)
            {
                if(!s.IsType("??GGA"))
                    throw Exception("Invalid type");
                if(s.size() != 15)
                    throw Exception("Wrong size");
                
                if (!s[6].empty())
                    quality = boost::lexical_cast<int>(s[6]);
                
                if (!s[7].empty())
                    sat_count = boost::lexical_cast<int>(s[7]);
                
                if (!s[8].empty())
                    hdop = boost::lexical_cast<float>(s[8]);
                
                if (!s[9].empty())
                    altitude = boost::lexical_cast<float>(s[9]);
                
                if (!s[11].empty())
                    geoid_height = boost::lexical_cast<float>(s[11]);
            }
            
        };
    }
}

#endif
