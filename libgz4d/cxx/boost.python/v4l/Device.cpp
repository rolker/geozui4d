// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/v4l/Device.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Device)
{
    class_<gz4d::v4l::Device, bases<>, std::shared_ptr<gz4d::v4l::Device> >("Device",init<std::string const &>())
    ;
}
