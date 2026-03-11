// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_NMEA_VDM_H
#define GZ4D_DATA_NMEA_VDM_H

#include <gz4d/data/nmea/Sentence.h>

namespace gz4d
{
    namespace nmea
    {
        struct VDM
        {
            int fragment_count;
            int fragment_number;
            std::string message_id;
            std::string channel;
            std::string payload;
            int pad_bits;
            
            VDM(Sentence const &s)
            {
                if(!s.IsType("??VD?"))
                    throw Exception("Invalid type");
                if(s.size() != 7)
                    throw Exception("Wrong size");
                fragment_count = boost::lexical_cast<int>(s[1]);
                fragment_number = boost::lexical_cast<int>(s[2]);
                message_id = s[3];
                channel = s[4];
                payload = s[5];
                pad_bits = boost::lexical_cast<int>(s[6]);
            }
        };
    }
}

#endif

            