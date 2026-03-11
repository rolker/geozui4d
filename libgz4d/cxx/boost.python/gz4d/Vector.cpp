// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Vector.h>
#include <gz4d/vector/ios.h>
#include <gz4d/vector/operations.h>

using namespace boost::python;

template <typename T, std::size_t N> inline T const &__getitem__(gz4d::Vector<T,N> &v, int i)
{
    if (i<0)
        i += N;
    if (i<0 || i >= N)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    return v[i];
}

template <typename T, std::size_t N> inline void __setitem__(gz4d::Vector<T,N> &v, int i, T const & value)
{
    if (i<0)
        i += N;
    if (i<0 || i >= N)
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    v[i] = value;
}

template <typename T, std::size_t N> inline int __len__(gz4d::Vector<T,N> &v) {return N;}

template <typename T, std::size_t N> inline gz4d::Vector<T,N> __addvec__(gz4d::Vector<T,N> &v, gz4d::Vector<T,N> &o) {return v+o;}
template <typename T, std::size_t N> inline gz4d::Vector<T,N> __addval__(gz4d::Vector<T,N> &v, T o) {return v+o;}

template <typename T, std::size_t N> inline gz4d::Vector<T,N> __subvec__(gz4d::Vector<T,N> &v, gz4d::Vector<T,N> &o) {return v-o;}
template <typename T, std::size_t N> inline gz4d::Vector<T,N> __subval__(gz4d::Vector<T,N> &v, T o) {return v-o;}

template <typename T, std::size_t N> inline gz4d::Vector<T,N> __mulvec__(gz4d::Vector<T,N> &v, gz4d::Vector<T,N> &o) {return v*o;}
template <typename T, std::size_t N> inline gz4d::Vector<T,N> __mulval__(gz4d::Vector<T,N> &v, T o) {return v*o;}

template <typename T, std::size_t N> inline gz4d::Vector<T,N> __divvec__(gz4d::Vector<T,N> &v, gz4d::Vector<T,N> &o) {return v/o;}
template <typename T, std::size_t N> inline gz4d::Vector<T,N> __divval__(gz4d::Vector<T,N> &v, T o) {return v/o;}

template<typename T, std::size_t N, typename IT> inline class_<gz4d::Vector<T,N> > &WrapVector(char const *name, IT const &i)
{
    return class_<gz4d::Vector<T,N> >(name,init<>())
            .def("__getitem__", __getitem__<T,N>, return_value_policy<copy_const_reference>() )
            .def("__setitem__", __setitem__<T,N>)
            .def("__len__", __len__<T,N>)
            .def("__add__", __addvec__<T,N>)
            .def("__add__", __addval__<T,N>)
            .def("__sub__", __subvec__<T,N>)
            .def("__sub__", __subval__<T,N>)
            .def("__mul__", __mulvec__<T,N>)
            .def("__mul__", __mulval__<T,N>)
            .def("__div__", __divvec__<T,N>)
            .def("__div__", __divval__<T,N>)
            .def(self_ns::str(self))
            .def(i)
            ;
}

template<typename T> inline void WrapVector2(char const *name)
{
    WrapVector<T,2>(name,init<T,T>());
}

template<typename T> inline void WrapVector3(char const *name)
{
    WrapVector<T,3>(name,init<T,T,T>());
}

template<typename T> inline void WrapVector4(char const *name)
{
    WrapVector<T,4>(name,init<T,T,T,T>());
}

template<typename T, std::size_t N> inline void WrapVectorNorm()
{
    def("norm", gz4d::norm<T,N>);
}


BOOST_PYTHON_MODULE(Vector)
{
    WrapVector2<float>("Vector2f");
    WrapVectorNorm<float,2>();
    WrapVector3<float>("Vector3f");
    WrapVectorNorm<float,3>();
    WrapVector4<float>("Vector4f");
    WrapVectorNorm<float,4>();
    WrapVector2<double>("Vector2d");
    WrapVectorNorm<double,2>();
    WrapVector3<double>("Vector3d");
    WrapVectorNorm<double,3>();
    WrapVector4<double>("Vector4d");
    WrapVectorNorm<double,4>();
    WrapVector3<uint16_t>("Vector3H");
    WrapVector<uint8_t,1>("Vector1B",init<uint8_t>());
    WrapVector3<uint8_t>("Vector3B");
    WrapVector4<uint8_t>("Vector4B");
    WrapVector2<std::size_t>("Vector2I");
}
