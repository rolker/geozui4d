// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Translation.h>

using namespace boost::python;

template<typename T> inline void WrapTranslation(char const *name)
{
    class_<gz4d::Translation<T>, boost::python::bases<gz4d::Point<T> > >(name,init<>())
            .def(init<gz4d::Vector<T,3> const &>())
            .def(init<T,T,T>())
            .def("GetMatrix", &gz4d::Translation<T>::GetMatrix)
            .def("GetInverseMatrix", &gz4d::Translation<T>::GetInverseMatrix)
            ;
}

BOOST_PYTHON_MODULE(Translation)
{
    WrapTranslation<double>("Translationd");
    WrapTranslation<float>("Translationf");
}

