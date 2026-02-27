#ifndef GZ4D_DATA_NMEA_HDG_H
#define GZ4D_DATA_NMEA_HDG_H

#include <gz4d/data/nmea/Sentence.h>

namespace gz4d
{
    namespace nmea
    {
        struct HDG
        {
            float magnetic_heading;
            float magnetic_deviation; ///< difference between compass north and magnetic north due to local effects.
            float magnetic_declination; ///< difference between true and magnetic north.

            HDG()
            :magnetic_heading(std::numeric_limits<float>::quiet_NaN()),magnetic_deviation(std::numeric_limits<float>::quiet_NaN()),magnetic_declination(std::numeric_limits<float>::quiet_NaN())
            {
            }
            
            HDG(Sentence const &s)
            :magnetic_heading(std::numeric_limits<float>::quiet_NaN()),magnetic_deviation(std::numeric_limits<float>::quiet_NaN()),magnetic_declination(std::numeric_limits<float>::quiet_NaN())
            {
                if(!s.IsType("??HDG"))
                    throw Exception("Invalid type");
                if(s.size() != 6)
                    throw Exception("Wrong size");
                
                if(!s[1].empty())
                {
                    magnetic_heading = boost::lexical_cast<float>(s[1]);
                }
                if(!s[2].empty())
                {
                    magnetic_deviation = boost::lexical_cast<float>(s[1]);
                    if (s[2] == "W")
                        magnetic_deviation = -magnetic_deviation;
                }
                
                if(!s[4].empty())
                {
                    magnetic_declination = boost::lexical_cast<float>(s[4]);
                    if (s[5] == "W")
                        magnetic_declination = -magnetic_declination;
                }
                
            }
            
        };
    }
}

#endif
