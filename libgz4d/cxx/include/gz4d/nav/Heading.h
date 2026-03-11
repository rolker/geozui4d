// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NAV_HEADING_H
#define GZ4D_NAV_HEADING_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>
#include <cmath>
#include <gz4d/Angle.h>

namespace gz4d
{
    namespace nav
    {
        template<typename T> struct Heading
        {
            typedef Angle<T,Degree,PositivePeriod> heading_type;
            heading_type heading;
            Heading():heading(Nan<T>()){}
            Heading(heading_type h):heading(h){}
        };
    }
}

#endif
