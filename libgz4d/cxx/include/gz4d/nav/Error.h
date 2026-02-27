#ifndef GZ4D_NAV_ERROR_H
#define GZ4D_NAV_ERROR_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    namespace nav
    {
        struct NoError
        {
        };

        template<typename T> struct Error
        {
            Point<T> variance;

            Error():variance(Nan<T>()){}
            Error(T x, T y, T z=Nan<T>()):variance(x,y,z){}
        };
    }
}

#endif
