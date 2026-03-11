// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_DATETIME_H
#define GZ4D_AIS_DATETIME_H

#include "Message.h"

namespace gz4d
{
    namespace ais
    {
        struct DateTime
        {
            Time timestamp;
            double longitude;
            double latitude;
            
            DateTime():longitude(0.0),latitude(0.0){}
            
            DateTime(Message const &m)
            {
                switch(m._message->message_id)
                {
                    case 4:
                    case 11:
                    {
                        std::shared_ptr<Ais4_11> ais4_11 = std::dynamic_pointer_cast<Ais4_11>(m._message);
                        timestamp = Time(ais4_11->year,ais4_11->month,ais4_11->day,ais4_11->hour,ais4_11->minute,ais4_11->second);
                        longitude = ais4_11->x;
                        latitude = ais4_11->y;
                    }
                }
            }
                 
                 
            bool operator!=(DateTime const &other) const
            {
                return timestamp != other.timestamp ||
                       longitude != other.longitude ||
                       latitude != other.latitude;
                
            }
            
            bool operator==(DateTime const &other) const
            {
                return !((*this) != other);
            }
        };
    }
}

#endif
