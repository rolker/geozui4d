// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_COLOR_H
#define GZ4D_GLO_COLOR_H

#include <gz4d/glo.h>
#include <gz4d/Color.h>


namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        inline void Color(gz4d::Color const &v)
        {
            glColor4fv(&(v[0]));
        }
    }
}

#endif

