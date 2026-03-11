// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_NORMAL_H
#define GZ4D_GLO_NORMAL_H

#include <gz4d/glo.h>
#include <gz4d/Vector.h>

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template <typename T> void Normal(Vector<T,3> const &v);

        template <> inline void Normal(Vector<GLfloat,3> const &v)
        {
            glNormal3fv(&v.front());
        }

        template <> inline void Normal(Vector<GLdouble,3> const &v)
        {
            glNormal3dv(&v.front());
        }

        template <> inline void Normal(Vector<GLbyte,3> const &v)
        {
            glNormal3bv(&v.front());
        }

        template <> inline void Normal(Vector<GLint,3> const &v)
        {
            glNormal3iv(&v.front());
        }

        template <> inline void Normal(Vector<GLshort,3> const &v)
        {
            glNormal3sv(&v.front());
        }
    }
}

#endif

