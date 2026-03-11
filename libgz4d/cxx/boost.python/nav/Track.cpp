// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/nav/Track.h>
#include <gz4d/thread/Consumer.h>

using namespace boost::python;

gz4d::nav::Fix::Ptr (gz4d::nav::Track::*call1)(gz4d::Time) = &gz4d::nav::Track::operator();
gz4d::nav::Fix::Ptr (gz4d::nav::Track::*call2)(gz4d::Time,gz4d::time::Duration) = &gz4d::nav::Track::operator();

BOOST_PYTHON_MODULE(Track)
{
    class_<gz4d::nav::Track, bases<gz4d::Producer<gz4d::nav::Fix::Ptr> >, std::shared_ptr<gz4d::nav::Track>, boost::noncopyable>("Track")
            .def(init<std::string const &>())
            .def("__call__", call1)
            .def("__call__", call2)
            ;

    gz4d::WrapConsumer<gz4d::nav::Track, std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >("TrackConsumer");
}
