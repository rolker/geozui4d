// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/BeamConfigurationManager.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(BeamConfigurationManager)
{
    class_<gz4d::Simrad::BeamConfigurationManager, bases<>, gz4d::Simrad::BeamConfigurationManager::Ptr >("BeamConfigurationManager",no_init)
            .def("GetConfigurations", &gz4d::Simrad::BeamConfigurationManager::GetConfigurations)
            .def("GetParameters", &gz4d::Simrad::BeamConfigurationManager::GetParameters, return_value_policy<copy_const_reference>())
            ;

}
