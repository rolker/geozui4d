#ifndef GZ4D_NAV_ATTITUDE_H
#define GZ4D_NAV_ATTITUDE_H

#include <gz4d/gz4d.h>
#include <gz4d/nav/Heading.h>

namespace gz4d
{
    namespace nav
    {
        template<typename T> struct Attitude: public Heading<T>
        {
            typedef Angle<T,Degree,ZeroCenteredPeriod> pitch_type;
            typedef Angle<T,Degree,ZeroCenteredPeriod> roll_type;
            
            pitch_type pitch;
            roll_type roll;
            Attitude():pitch(Nan<T>()),roll(Nan<T>()){}
            Attitude(typename Heading<T>::heading_type h, pitch_type p, roll_type r):Heading<T>(h),pitch(p),roll(r){}
        };
    }
}

#endif
