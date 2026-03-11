// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Point.h>

using namespace boost::python;

template<typename T> void WrapPoint(char const *name)
{
    boost::python::class_<gz4d::Point<T>, boost::python::bases<gz4d::Vector<T,3> > >(name)
        .def(boost::python::init<T, T, T>())
        .def(boost::python::init<gz4d::Vector<T,3> const &>())
        .def(boost::python::init<gz4d::Vector<T,4> const &>())
        ;
}

BOOST_PYTHON_MODULE(Point)
{
    WrapPoint<float>("Pointf");
    WrapPoint<double>("Pointd");
}
