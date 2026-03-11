// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_CIRCLE_H
#define GZ4D_DATA_CIRCLE_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct Circle
    {
        T center;
        typename T::value_type radius;
        Circle():radius(0.0){}
        Circle(T const &c, typename T::value_type r):center(c),radius(r){}
    };
}

#endif
