// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_CRFBASE_H
#define GZ4D_GEO_CRFBASE_H

#include <gz4d/gz4d.h>
#include <string>

namespace gz4d
{
    namespace geo
    {
        /// Loosely based on ISO 19111
        
        /// Base for CRF related type structures.
        template <typename T> struct CRFBase
        {
            static const std::string &label()
            {
                return label(T());
            }
        };
        
    }
}

#endif
