#ifndef GZ4D_NAV_IOS_H
#define GZ4D_NAV_IOS_H

#include <gz4d/ios.h>
#include <gz4d/time/ios.h>
#include <gz4d/nav/Fix.h>

namespace gz4d
{
    namespace nav
    {
        std::ostream& operator<<(std::ostream &str, Fix const &f)
        {
            return str << f.timestamp << " " << std::fixed << std::setprecision(6) << f.latitude.value() << ", " << f.longitude.value() << "\talt: " << f.altitude <<  " h: " << f.heading << " p: " << f.pitch << " r: " << f.roll;
        }
    }
}

#endif
