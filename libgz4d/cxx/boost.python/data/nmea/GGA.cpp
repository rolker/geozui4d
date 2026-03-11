// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/GGA.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(GGA)
{
    class_<gz4d::nmea::GGA, bases<gz4d::nmea::Time, gz4d::nmea::LatLon> >("GGA", init<gz4d::nmea::Sentence const&>())
            .def(init<>())
            .def_readwrite("quality", &gz4d::nmea::GGA::quality)
            .def_readwrite("sat_count", &gz4d::nmea::GGA::sat_count)
            .def_readwrite("hdop", &gz4d::nmea::GGA::hdop)
            .def_readwrite("altitude", &gz4d::nmea::GGA::altitude)
            .def_readwrite("geoid_height", &gz4d::nmea::GGA::geoid_height)
            .def_readwrite("dgps_age", &gz4d::nmea::GGA::dgps_age)
            .def_readwrite("dgps_id", &gz4d::nmea::GGA::dgps_id)
            ;
}
