// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_MULTMATRIX_H
#define GZ4D_GLO_MULTMATRIX_H

#include <gz4d/glo.h>
#include <gz4d/Matrix.h>


namespace gz4d
{
    namespace glo
    {

        //template <typename T> void MultMatrix(Matrix<T,4,4> const &m);

        //template <> inline 
        /// \ingroup glo
        inline void MultMatrix(Matrix<float,4,4> const &m)
        {
            glMultMatrixf(&m.front());
        }

        //template <> inline 
        /// \ingroup glo
        inline void MultMatrix(Matrix<double,4,4> const &m)
        {
            glMultMatrixd(&m.front());
        }
    }
}

#endif
