// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/Platform.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(Platform)
{
    class_<gz4d::Simrad::Platform, bases<>, std::shared_ptr<gz4d::Simrad::Platform> >("Platform",init<gz4d::Simrad::ParameterManager::Ptr &>())
            .def("GetParameters", &gz4d::Simrad::Platform::GetParameters, return_value_policy<copy_const_reference>())
            ;

}
