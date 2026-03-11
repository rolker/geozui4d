// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/time.h>
#include <gz4d/thread/Producer.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Time)
{

    class_<gz4d::Time>("Time")
            .def(init<double>())
            //.def(init<std::string const &>())
            //.def(float_(self))
            .def("secondsSinceEpoch", &gz4d::Time::secondsSinceEpoch)
            .def("Now", &gz4d::time::Now)
            .staticmethod("Now")
    ;

    gz4d::WrapProducer<gz4d::Time>("TimeProducer");
    gz4d::WrapThreadedQueue<gz4d::Time>("TimeThreadedQueue");

}
