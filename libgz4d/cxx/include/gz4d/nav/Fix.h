#ifndef GZ4D_NAV_FIX_H
#define GZ4D_NAV_FIX_H

#include <gz4d/gz4d.h>
#include <gz4d/geo/GeodeticPoint.h>
#include <gz4d/nav/Attitude.h>
#include <gz4d/time.h>

#include <gz4d/data/binary/Packet.h>
#include <gz4d/socket/Socket.h>


namespace gz4d
{

    namespace nav
    {
        struct Fix: public geo::GeodeticPoint<double>, public Attitude<double>
        {
            Time timestamp;

            Fix(){}
            Fix(binary::Navigation::Position6DOF::Geographic const &n)
            {
                timestamp = n.timestamp;
                latitude = n.latitude;
                longitude = n.longitude;
                altitude = n.z;
                heading = n.heading;
                pitch = n.pitch;
                roll = n.roll;
            }
            
            bool IsComplete()
            {
                return (timestamp.valid() && !IsNan(latitude) && !IsNan(longitude) && !IsNan(altitude) && !IsNan(heading) && !IsNan(pitch) && !IsNan(roll));
            }
            
            typedef std::shared_ptr<Fix> Ptr;
            
            void SendVia(Socket &p)
            {
                gz4d::binary::Navigation::Position6DOF::Geographic packet = *this;
                p.Send(static_cast<binary::Packet const*>(&packet));
            }
            
            operator binary::Navigation::Position6DOF::Geographic() const
            {
                gz4d::binary::Navigation::Position6DOF::Geographic packet;
                packet.timestamp = timestamp;
                packet.latitude = latitude.value();
                packet.longitude = longitude.value();
                packet.z = altitude;
                packet.heading = heading.value();
                packet.pitch = pitch.value();
                packet.roll = roll.value();
                return packet;
            }
            
            
        };
    }
}


#endif
