// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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