// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_VECTOR_H
#define GZ4D_VECTOR_H

#include <gz4d/gz4d.h>
#include <boost/static_assert.hpp>
#include <cstring>
#include <utility>

namespace gz4d
{
    /// Base type for n-dimensional vectors.
    /// \ingroup base
    template <typename T, std::size_t N>
    class Vector
    {
        public:
            static const std::size_t _size = N;
        protected:
            T values[N];
        public:
            Vector();
            Vector(Vector<T,N> const &v);
            template<typename OT> Vector(Vector<OT,N> const &v);
            template<typename OT, std::size_t ON> Vector(Vector<OT,ON> const &v, std::size_t i);
            explicit Vector(T val);
            Vector(T v1, T v2);
            Vector(T v1, T v2, T v3);
            Vector(T v1, T v2, T v3, T v4);
            Vector(const T v[N]);
            
            template <typename VI> Vector(const std::pair<VI,VI> &iterators)
            {
                T* vp = values;
                for(VI v = iterators.first; v != iterators.second; ++v)
                    *vp++ = *v;
            }

            Vector<T,N> &operator=(Vector<T,N> const &rvalue);

            bool operator==(Vector<T,N> const &rvalue) const;
            bool operator!=(Vector<T,N> const &rvalue) const {return !(*this == rvalue);}

            Vector<T,N> const &operator+=(Vector<T,N> const &rvalue);
            Vector<T,N> const &operator+=(T rvalue);
            Vector<T,N> const &operator-=(Vector<T,N> const &rvalue);
            Vector<T,N> const &operator-=(T rvalue);
            Vector<T,N> const &operator*=(Vector<T,N> const &rvalue);
            Vector<T,N> const &operator*=(T rvalue);
            Vector<T,N> const &operator/=(Vector<T,N> const &rvalue);
            Vector<T,N> const &operator/=(T rvalue);

            Vector<T,N> operator-() const;

            template <typename RT> Vector<T,N> operator+(RT const &rvalue) const {return Vector<T,N>(*this) += rvalue;}
            template <typename RT> Vector<T,N> operator-(RT const &rvalue) const {return Vector<T,N>(*this) -= rvalue;}
            template <typename RT> Vector<T,N> operator*(RT const &rvalue) const {return Vector<T,N>(*this) *= rvalue;}
            template <typename RT> Vector<T,N> operator/(RT const &rvalue) const {return Vector<T,N>(*this) /= rvalue;}

            /// Dot product
            /// theta = acos((a.b)/(|a||b|)) where |a| means norm(a)
            T dot(Vector<T,N> const &rvalue) const;
            
            /// Returns length(1D), area(2D), volume(3D) or higher dim equivalent.
            T volume() const
            {
                T ret = values[0];
                for(int i = 1; i < N; ++i)
                    ret *= values[i];
                return ret;
            }

            T &operator[](std::size_t index){return values[index];}
            T const &operator[](std::size_t index) const{return values[index];}

            T &front() {return *values;}
            T const &front() const {return *values;}

            static std::size_t size() {return N;}

            typedef T value_type;
    };

    template <typename T, std::size_t N> inline Vector<T,N>::Vector()
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] = 0;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(Vector<T,N> const &v)
    {
        //for(std::size_t i = 0; i < N; ++i)
            //values[i] = v[i];
        memcpy(&values,&v.values,sizeof(T)*N);
    }

    template <typename T, std::size_t N> template<typename OT> inline Vector<T,N>::Vector(Vector<OT,N> const &v)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] = v[i];
    }

    template <typename T, std::size_t N> template<typename OT, std::size_t ON> inline Vector<T,N>::Vector(Vector<OT,ON> const &v, std::size_t offset)
    {
        for(std::size_t i = 0; i < N && i < ON+offset; ++i)
            values[i] = v[i+offset];
        for(std::size_t i = ON+offset; i < N; ++i)
            values[i] = 0;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(T val)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] = val;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(T v1, T v2)
    {
        BOOST_STATIC_ASSERT(N == 2);
        values[0] = v1;
        values[1] = v2;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(T v1, T v2, T v3)
    {
        BOOST_STATIC_ASSERT(N == 3);
        values[0] = v1;
        values[1] = v2;
        values[2] = v3;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(T v1, T v2, T v3, T v4)
    {
        BOOST_STATIC_ASSERT(N == 4);
        values[0] = v1;
        values[1] = v2;
        values[2] = v3;
        values[3] = v4;
    }

    template <typename T, std::size_t N> inline Vector<T,N>::Vector(const T v[N])
    {
        for (std::size_t i = 0; i < N; i++)
            values[i] = v[i];
    }

    template<typename T, std::size_t N> inline Vector<T,N> &Vector<T,N>::operator=(Vector<T,N> const &rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] = rvalue[i];
        return *this;
    }

    template <typename T, std::size_t N> inline bool Vector<T,N>::operator==(Vector<T,N> const &rvalue) const
    {
        bool ret=true;
        for(std::size_t i = 0; ret && i < N; ++i)
            ret = values[i] == rvalue[i];
        return ret;
    }


    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator+=(Vector<T,N> const &rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] += rvalue[i];
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator+=(T rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] += rvalue;
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator-=(Vector<T,N> const &rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] -= rvalue[i];
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator-=(T rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] -= rvalue;
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator*=(Vector<T,N> const &rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] *= rvalue[i];
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator*=(T rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] *= rvalue;
        return *this;
    }


    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator/=(Vector<T,N> const &rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] /= rvalue[i];
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> const &Vector<T,N>::operator/=(T rvalue)
    {
        for(std::size_t i = 0; i < N; ++i)
            values[i] /= rvalue;
        return *this;
    }

    template <typename T, std::size_t N> inline Vector<T,N> Vector<T,N>::operator-() const
    {
        Vector<T,N> ret;
        for(std::size_t i = 0; i < N; ++i)
            ret.values[i] = -values[i];
        return ret;
    }

    template <typename T, std::size_t N> inline T Vector<T,N>::dot(Vector<T,N> const &rvalue) const
    {
        T sum = 0;

        for(std::size_t i = 0; i < N; ++i)
            sum += values[i] * rvalue[i];

        return sum;
    }
}

template <typename T, std::size_t N> gz4d::Vector<T,N> operator+(T l, gz4d::Vector<T,N> const &r)
{
    return r+l;
}

template <typename T, std::size_t N> gz4d::Vector<T,N> operator-(T l, gz4d::Vector<T,N> const &r)
{
    return -r+l;
}

template <typename T, std::size_t N> gz4d::Vector<T,N> operator*(T l, gz4d::Vector<T,N> const &r)
{
    return r*l;
}



#endif
