// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/geo/Position.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Position)
{
    class_<gz4d::geo::Position<double> >("Position")
            .def(init<double,double>())
            .def_readwrite("latitude", &gz4d::geo::Position<double>::latitude)
            .def_readwrite("longitude", &gz4d::geo::Position<double>::longitude)
            ;
}
