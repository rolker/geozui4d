// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IOS_H
#define GZ4D_IOS_H

#include "Angle.h"
#include "Box.h"
#include "Color.h"

namespace gz4d
{
    template <typename T, typename PT, typename RT> std::ostream &operator<<(std::ostream &str, Angle<T,PT,RT> const &a)
    {
        return str << a.value();
    }
    
    template <typename T> std::ostream &operator<<(std::ostream &str, Box<T> const &b)
    {
        return str << "[" << b.getMin() << " - " << b.getMax() << "]";
    }
    
    inline std::istream &operator>>(std::istream &str, Color &c)
    {
        for (std::size_t i = 0; i < 4; ++i)
        {
            if (!(str >> c[i]))
            {
                if (i<3)
                    throw gz4d::ExtractionException(i, std::string("Incomplete vector extraction"));
            }
        }
        return str;
    }
}

#endif
