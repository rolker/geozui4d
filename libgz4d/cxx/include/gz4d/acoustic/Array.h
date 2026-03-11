// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ACOUSTIC_ARRAY_H
#define GZ4D_ACOUSTIC_ARRAY_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Element.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Abstract Base for a collection of Elements.
        /// \ingroup acoustic
        class Array : public Element
        {};
    }
}

#endif
