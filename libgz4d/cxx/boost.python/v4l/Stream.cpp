// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/v4l/Stream.h>


boost::rational<uint32_t> (gz4d::v4l::Stream::*framePeriod0)() = &gz4d::v4l::Stream::framePeriod;
boost::rational<uint32_t> (gz4d::v4l::Stream::*framePeriod1)(boost::rational<uint32_t>) = &gz4d::v4l::Stream::framePeriod;

using namespace boost::python;

BOOST_PYTHON_MODULE(Stream)
{

    gz4d::WrapProducer<gz4d::binary::Wrapper<gz4d::binary::Image::Frame> >("FrameProducer");
    gz4d::WrapThreadedQueue<gz4d::binary::Wrapper<gz4d::binary::Image::Frame> >("FrameThreadedQueue");

    class_<gz4d::v4l::Stream, bases<gz4d::Producer<gz4d::binary::Wrapper<gz4d::binary::Image::Frame> > >, std::shared_ptr<gz4d::v4l::Stream>, boost::noncopyable >("Stream",init<gz4d::v4l::Device const &, uint32_t, uint32_t, uint32_t>())
        .def("Start", &gz4d::v4l::Stream::Start)
        .def("Stop", &gz4d::v4l::Stream::Stop)
        .def("framePeriod", framePeriod0)
        .def("framePeriod", framePeriod1)
        .def("width", &gz4d::v4l::Stream::width)
        .def("height", &gz4d::v4l::Stream::height)
    ;
}

