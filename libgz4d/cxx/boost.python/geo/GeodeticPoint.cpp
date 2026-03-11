// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/geo/GeodeticPoint.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(GeodeticPoint)
{
    class_<gz4d::geo::GeodeticPoint<double>, bases<gz4d::geo::Position<double> > >("GeodeticPoint")
            .def_readwrite("altitude", &gz4d::geo::GeodeticPoint<double>::altitude)
            ;
}
