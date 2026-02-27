#ifndef GZ4D_GLO_LOADMATRIX_H
#define GZ4D_GLO_LOADMATRIX_H

#include <gz4d/glo.h>
#include <gz4d/Matrix.h>


namespace gz4d
{
    namespace glo
    {

        /// \ingroup glo
        template <typename T> void LoadMatrix(Matrix<T,4,4> const &m);

        template <> inline void LoadMatrix(Matrix<float,4,4> const &m)
        {
            glLoadMatrixf(&m.front());
        }

        template <> inline void LoadMatrix(Matrix<double,4,4> const &m)
        {
            glLoadMatrixd(&m.front());
        }
    }
}

#endif

