// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <boost/math/quaternion.hpp>
#include <boost/rational.hpp>

#include <gz4d/gz4d.h>

using namespace boost::python;

template <typename T> inline void WrapQuaternion(char const *name)
{
    class_<boost::math::quaternion<T> >(name,init<>())
            .def(init<T,T,T,T>())
            
            ;
}

template <typename T> inline void WrapRational(char const *name)
{
    class_<boost::rational<T> >(name,init<>())
    .def(init<T>())
    .def(init<T,T>())
    .def("numerator", &boost::rational<T>::numerator,return_value_policy<copy_const_reference>())
    .def("denominator", &boost::rational<T>::denominator,return_value_policy<copy_const_reference>())
    ;
}

BOOST_PYTHON_MODULE(Boost)
{
    WrapQuaternion<double>("quaterniond");
    WrapQuaternion<float>("quaternionf");

    WrapRational<uint32_t>("rational32");
}


