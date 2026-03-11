// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_TRANSLATE_H
#define GZ4D_GLO_TRANSLATE_H

#include <gz4d/glo.h>
#include <gz4d/Vector.h>


namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template <typename T, std::size_t N> void Translate(Vector<T,N> const &v);

        template <> inline void Translate(Vector<float,3> const &v)
        {
            glTranslatef(v[0],v[1],v[2]);
        }

        template <> inline void Translate(Vector<double,3> const &v)
        {
            glTranslated(v[0],v[1],v[2]);
        }
    }
}

#endif

