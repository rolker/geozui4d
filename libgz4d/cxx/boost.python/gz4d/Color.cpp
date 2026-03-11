// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Color.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Color)
{
    class_<gz4d::Color, boost::python::bases<gz4d::Vector<float,4> > >("Color")
            ;
    def("Black", &gz4d::Black);
    def("White", &gz4d::White);
    def("Grey", &gz4d::Grey);
    def("Red", &gz4d::Red);
    def("Green", &gz4d::Green);
    def("Blue", &gz4d::Blue);
}

