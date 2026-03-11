// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_VERTEXPOINTER_H
#define GZ4D_GLO_VERTEXPOINTER_H

#include <gz4d/glo.h>

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template <typename T> void VertexPointer(std::vector<T> const &data)
        {
            glVertexPointer(T::size(), glo::Type<typename T::value_type>(),0,&data.front());
        }
    }
}

#endif
