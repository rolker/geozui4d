// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_POINT_H
#define GZ4D_GEO_POINT_H

#include <gz4d/gz4d.h>
#include <gz4d/Point.h>

namespace gz4d
{
    namespace geo
    {
        template<typename T, typename RF> class Point: public gz4d::Point<T>
        {
            public:

                Point(){}
                Point(T x, T y, T z):gz4d::Point<T>(x,y,z){}
                explicit Point(gz4d::Point<T> const &op):gz4d::Point<T>(op){}
                using gz4d::Point<T>::operator=;

                template<typename OT, typename ORF> explicit Point(Point<OT,ORF> const&op)
                {
                    *this = typename RF::coordinate_type()(op);
                }
        };

    }
}

#endif
