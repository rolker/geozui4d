// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Matrix.h>
#include <gz4d/matrix/ios.h>

using namespace boost::python;


template <typename T, std::size_t M, std::size_t N> inline T const &__getitem__(gz4d::Matrix<T,M,N> &m, tuple t)
{
    int i = extract<int>(t[0]);
    int j = extract<int>(t[1]);

    if (i<0)
        i += M;
    if (i<0 || i >= M)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    if (j<0)
        j += N;
    if (j<0 || j >= M)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    return m(i,j);
}

template <typename T, std::size_t M, std::size_t N> inline void __setitem__(gz4d::Matrix<T,M,N> &m, tuple t, T const & value)
{
    int i = extract<int>(t[0]);
    int j = extract<int>(t[1]);

    if (i<0)
        i += M;
    if (i<0 || i >= M)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    if (j<0)
        j += N;
    if (j<0 || j >= M)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    m(i,j) = value;
}

template <typename T, std::size_t M, std::size_t N> inline gz4d::Vector<T,M> __mulvec__(gz4d::Matrix<T,M,N> &m, gz4d::Vector<T,N> &v) {return m*v;}


template<typename T, std::size_t M, std::size_t N> inline void WrapMatrix(char const *name)
{
    class_<gz4d::Matrix<T,M,N> >(name,init<>())
        .def(self_ns::str(self))
        .def(self * gz4d::Matrix<T,N,N>())
        .def("__getitem__", __getitem__<T,M,N>, return_value_policy<copy_const_reference>() )
        .def("__setitem__", __setitem__<T,M,N>)
        .def("__mul__", __mulvec__<T,M,N>)

        .def("Identity", &gz4d::Matrix<T,M,N>::Identity)
        .staticmethod("Identity")
        ;
}

template<typename T> inline void WrapFrustum(char const *name)
{
    def(name, &gz4d::Frustum<T>);
}

BOOST_PYTHON_MODULE(Matrix)
{
    WrapMatrix<double,4,4>("Matrix4d");
    WrapMatrix<float,4,4>("Matrix4f");
    WrapFrustum<double>("Frustumd");
    WrapFrustum<float>("Frustumf");
}
