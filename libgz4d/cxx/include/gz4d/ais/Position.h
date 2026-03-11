// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_POSITION_H
#define GZ4D_AIS_POSITION_H

#include "Message.h"
#include <gz4d/Angle.h>
#include <gz4d/util/Utilities.h>

using namespace libais;

namespace gz4d
{
    namespace ais
    {
        struct Position
        {
            double longitude;
            double latitude;
            double speedOverGround;
            double rateOfTurn;
            double courseOverGround;
            double heading;
            
            Position():longitude(0.0),latitude(0.0),speedOverGround(0.0),rateOfTurn(0.0),courseOverGround(0.0),heading(0.0){}
            
            Position(Message const &m)
            {
                switch(m._message->message_id)
                {
                    case 1:
                    case 2:
                    case 3:
                    {
                        std::shared_ptr<Ais1_2_3> ais123 = std::dynamic_pointer_cast<Ais1_2_3>(m._message);
                        longitude = ais123->x;
                        latitude = ais123->y;
                        speedOverGround = ais123->sog;
                        rateOfTurn = ais123->rot;
                        courseOverGround = ais123->cog;
                        heading = ais123->true_heading;
                    }
                    break;
                    case 18:
                    {
                        std::shared_ptr<Ais18> ais18 = std::dynamic_pointer_cast<Ais18>(m._message);
                        longitude = ais18->x;
                        latitude = ais18->y;
                        speedOverGround = ais18->sog;
                        rateOfTurn = Nan<float>();
                        courseOverGround = ais18->cog;
                        heading = ais18->true_heading;
                    }
                }
            }
            
            bool operator!=(Position const &other) const
            {
                return longitude != other.longitude ||
                       latitude != other.latitude ||
                       speedOverGround != other.speedOverGround ||
                       rateOfTurn != other.rateOfTurn ||
                       courseOverGround != other.courseOverGround ||
                       heading != other.heading;
            }
            
            bool operator==(Position const &other) const
            {
                return !((*this) != other);
            }
        };

        inline Position interpolate(Position const &p1, Position const &p2, double p)
        {
            Angle<double,Degree,ZeroCenteredPeriod> lon1 = p1.longitude, lon2 = p2.longitude,
                lat1 = p1.latitude, lat2 = p2.latitude;
            Angle<double,Degree,PositivePeriod> cog1 = p1.courseOverGround, cog2 = p2.courseOverGround,
                h1 = p1.heading, h2 = p2.heading;
            Position ret;
            ret.latitude = interpolate(lat1,lat2,p).value();
            ret.longitude = interpolate(lon1,lon2,p).value();
            ret.speedOverGround = gz4d::interpolate(p1.speedOverGround,p2.speedOverGround,p);
            ret.rateOfTurn = gz4d::interpolate(p1.rateOfTurn,p2.rateOfTurn,p);
            ret.courseOverGround = interpolate(cog1,cog2,p).value();
            ret.heading = interpolate(h1,h2,p).value();
            return ret;
        }
    }
}

#endif
