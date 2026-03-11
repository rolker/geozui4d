// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_LATLON_H
#define GZ4D_DATA_NMEA_LATLON_H

#include <gz4d/data/nmea/Sentence.h>

namespace gz4d
{
    namespace nmea
    {
        struct LatLon
        {
            double latitude;
            double longitude;

            LatLon()
            :latitude(std::numeric_limits<double>::quiet_NaN()),longitude(std::numeric_limits<double>::quiet_NaN())
            {
            }
            
            LatLon(Sentence const &s, int index)
            :latitude(std::numeric_limits<double>::quiet_NaN()),longitude(std::numeric_limits<double>::quiet_NaN())
            {
                if(s.size() > index+3)
                {
                    if(s[index].size() >= 4)
                    {
                        double lat = boost::lexical_cast<int>(s[index].substr(0,2));
                        lat += boost::lexical_cast<double>(s[index].substr(2))/60.0;
                        if(s[index+1] == "N")
                            latitude = lat;
                        else if(s[index+1] == "S")
                            latitude = -lat;
                    }
                    
                    if(s[index+2].size() >= 5)
                    {
                        double lon = boost::lexical_cast<int>(s[index+2].substr(0,3));
                        lon += boost::lexical_cast<double>(s[index+2].substr(3))/60.0;
                        if(s[index+3] == "E")
                            longitude = lon;
                        else if(s[index+3] == "W")
                            longitude = -lon;
                    }
                }
            }
            
        };
    }
}

#endif
