// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/HDG.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(HDG)
{
    class_<gz4d::nmea::HDG >("HDG", init<gz4d::nmea::Sentence const&>())
            .def(init<>())
            .def_readwrite("magnetic_heading", &gz4d::nmea::HDG::magnetic_heading)
            .def_readwrite("magnetic_deviation", &gz4d::nmea::HDG::magnetic_deviation)
            .def_readwrite("magnetic_declination", &gz4d::nmea::HDG::magnetic_declination)
            ;
}
