#ifndef GZ4D_DATA_LINEARRING_H
#define GZ4D_DATA_LINEARRING_H

#include <gz4d/data/Line.h>

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct LinearRing: public Line<T>
    {
        bool clockwise;
        LinearRing():clockwise(false){}
    };
}

#endif