// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_LINE_H
#define GZ4D_DATA_LINE_H


#include <gz4d/gz4d.h>
#include <vector>
#include <memory>

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct Line
    {
        std::vector<T> data;
    };
}


#endif
