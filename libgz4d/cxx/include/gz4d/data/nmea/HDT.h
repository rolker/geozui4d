// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_HDT_H
#define GZ4D_DATA_NMEA_HDT_H

#include <gz4d/data/nmea/Sentence.h>

namespace gz4d
{
    namespace nmea
    {
        struct HDT
        {
            float heading;
            char reference;
            HDT()
            :heading(std::numeric_limits<float>::quiet_NaN()),reference(0)
            {
            }
            
            HDT(Sentence const &s)
            :heading(std::numeric_limits<float>::quiet_NaN()),reference(0)
            {
                if(!s.IsType("??HDT"))
                    throw Exception("Invalid type");
                if(s.size() != 3)
                    throw Exception("Wrong size");
                
                if(!s[1].empty())
                {
                    heading = boost::lexical_cast<float>(s[1]);
                    if(s[2].size()==1)
                        reference = s[2][0];
                }
            }
            
        };
    }
}

#endif
