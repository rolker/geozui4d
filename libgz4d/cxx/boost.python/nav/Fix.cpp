// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/nav/Fix.h>
#include <gz4d/nav/ios.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/socket/UDPSender.h>
#include <gz4d/thread/Consumer.h>

using namespace boost::python;

std::string __str__(gz4d::nav::Fix const &f)
{
    std::stringstream ret;
    ret << f;
    return ret.str();
}


BOOST_PYTHON_MODULE(Fix)
{
    class_<gz4d::nav::Fix, bases<gz4d::geo::GeodeticPoint<double>,gz4d::nav::Attitude<double> >, std::shared_ptr<gz4d::nav::Fix> >("Fix")
            .def_readwrite("timestamp", &gz4d::nav::Fix::timestamp)
            .def("IsComplete", &gz4d::nav::Fix::IsComplete)
            .def("__str__", __str__)
            ;

    gz4d::WrapProducer<gz4d::nav::Fix::Ptr>("FixProducer");
    gz4d::WrapThreadedQueue<gz4d::nav::Fix::Ptr>("FixThreadedQueue");

    gz4d::WrapUDPSender<gz4d::nav::Fix::Ptr>("FixUDPSender");
    gz4d::WrapConsumer<gz4d::UDPSender<gz4d::nav::Fix::Ptr>,gz4d::nav::Fix::Ptr>("FixUDPSenderConsumer");
}
