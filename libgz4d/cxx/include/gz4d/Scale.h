#ifndef GZ4D_SCALE_H
#define GZ4D_SCALE_H

#include <gz4d/gz4d.h>
#include <gz4d/Vector.h>
#include <gz4d/Matrix.h>

namespace gz4d
{
    /// \ingroup base
    template<typename T> class Scale: public Vector<T,3>
    {
        public:
            Scale();
            Scale(T s);
            Scale(Vector<T,3> const &v);
            Scale(T x, T y, T z);
            Matrix<T,4,4> GetMatrix() const;
            Matrix<T,4,4> GetInverseMatrix() const;
    };

    template<typename T> inline Scale<T>::Scale()
    :Vector<T,3>(1.0)
    {
    }

    template<typename T> inline Scale<T>::Scale(T s)
    :Vector<T,3>(s)
    {
    }

    template<typename T> inline Scale<T>::Scale(Vector<T,3> const &v)
    :Vector<T,3>(v)
    {
    }

    template<typename T> inline Scale<T>::Scale(T x, T y, T z)
    :Vector<T,3>(x,y,z)
    {
    }

    template<typename T>
            inline Matrix<T,4,4> Scale<T>::GetMatrix() const
    {
        Matrix<T,4,4> ret;
        for(unsigned i = 0; i < 3; ++i)
            ret(i,i) = (*this)[i];
        ret(3,3) = 1;
        return ret;
    }

    template<typename T>
            inline Matrix<T,4,4> Scale<T>::GetInverseMatrix() const
    {
        Matrix<T,4,4> ret;
        for(unsigned i = 0; i < 3; ++i)
            ret(i,i) = 1.0/(*this)[i];
        ret(3,3) = 1;
        return ret;
    }

}

#endif
