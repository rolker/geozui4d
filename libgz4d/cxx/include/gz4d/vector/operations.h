#ifndef GZ4D_VECTOR_OPERATIONS_H
#define GZ4D_VECTOR_OPERATIONS_H

#include <gz4d/Vector.h>

namespace gz4d
{
    template <typename T, std::size_t N> T norm2(Vector<T, N> const &v)
    {
        T ret = v[0]*v[0];
        for(std::size_t i = 1; i < N; ++i)
            ret += v[i]*v[i];
        return ret;
    }

    template <typename T, std::size_t N> T norm(Vector<T, N> const &v)
    {
        return sqrt(norm2(v));
    }

    template <typename T, std::size_t N> Vector<T, N> normalize(Vector<T, N> const &v)
    {
        return v/norm(v);
    }

    template <typename T> Vector<T, 3> cross(Vector<T, 3> const &a, Vector<T, 3> const &b)
    {
        return Vector<T, 3>(a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]);
    }

    template <typename T> Vector<T, 3> cross(Vector<T, 2> const &a, Vector<T, 2> const &b)
    {
        return Vector<T,3>(0,0,(a[0]*b[1])-(a[1]*b[0]));
    }
}

template <typename T, std::size_t N> gz4d::Vector<T, N> ceil(gz4d::Vector<T, N> const &v)
{
    gz4d::Vector<T, N>  ret;
    for(std::size_t i = 0; i < N; ++i)
        ret[i] = ceil(v[i]);
    return ret;
}

template <typename T, std::size_t N> gz4d::Vector<T, N> floor(gz4d::Vector<T, N> const &v)
{
    gz4d::Vector<T, N>  ret;
    for(std::size_t i = 0; i < N; ++i)
        ret[i] = floor(v[i]);
    return ret;
}

#endif
