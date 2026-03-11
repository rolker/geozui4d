// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TRANSLATION_H
#define GZ4D_TRANSLATION_H

#include <gz4d/gz4d.h>
#include <gz4d/Point.h>
#include <gz4d/Units.h>
#include <gz4d/Matrix.h>
namespace gz4d
{
    /// \ingroup base
    template<typename T> class Translation: public Point<T>
    {
        public:
            Translation();
            Translation(Vector<T,3> const & v);
            Translation(T x, T y, T z);
            Matrix<T,4,4> GetMatrix() const;
            Matrix<T,4,4> GetInverseMatrix() const;
            using Point<T>::operator=;
    };

    template<typename T> inline Translation<T>::Translation()
    {}

    template<typename T> inline Translation<T>::Translation(Vector<T,3> const & v)
    :Point<T>(v)
    {}

    template<typename T> inline Translation<T>::Translation(T x, T y, T z)
    :Point<T>(x,y,z)
    {}

    template<typename T> inline Matrix<T,4,4> Translation<T>::GetMatrix() const
    {
        Matrix<T,4,4> ret = Matrix<T,4,4>::Identity();
        ret(0,3) = (*this)[0];
        ret(1,3) = (*this)[1];
        ret(2,3) = (*this)[2];
        return ret;
    }

    template<typename T> inline Matrix<T,4,4> Translation<T>::GetInverseMatrix() const
    {
        Matrix<T,4,4> ret = Matrix<T,4,4>::Identity();
        ret(0,3) = -(*this)[0];
        ret(1,3) = -(*this)[1];
        ret(2,3) = -(*this)[2];
        return ret;
    }

}

#endif
