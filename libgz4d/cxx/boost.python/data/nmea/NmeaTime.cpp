// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/Time.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(NmeaTime)
{
    class_<gz4d::nmea::Time>("Time")
            .def_readwrite("hour", &gz4d::nmea::Time::hour)
            .def_readwrite("minute", &gz4d::nmea::Time::minute)
            .def_readwrite("second", &gz4d::nmea::Time::second)
            ;
}
