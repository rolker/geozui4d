#ifndef GZ4D_UTIL_SINC_H
#define GZ4D_UTIL_SINC_H

#include <gz4d/gz4d.h>
#include <boost/math/special_functions.hpp>
#include <limits>

namespace gz4d
{
    template <typename T> T inv_sinc_pi(T y)
    {
        if (y >= 1.0)
            return 0;
        if (y < 0.0)
            return boost::math::constants::pi<T>();

        T x = M_PI*(1.0-y);
        T yt = boost::math::sinc_pi(x);
        while (fabs(yt-y) > std::numeric_limits<T>::epsilon())
        {
            x += yt-y;
            yt = boost::math::sinc_pi(x);
        }
        return x;
    }
}

#endif
