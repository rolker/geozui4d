// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/Client.h>

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(connect_overloads,Connect,0,2)

BOOST_PYTHON_MODULE(Client)
{

    class_<gz4d::Simrad::Client, bases<>, std::shared_ptr<gz4d::Simrad::Client>, boost::noncopyable >("Client",init<gz4d::Simrad::Server const &>())
            .def("Connect", &gz4d::Simrad::Client::Connect, connect_overloads())
            .def("GetPlatform", &gz4d::Simrad::Client::GetPlatform,return_value_policy<copy_non_const_reference>())
            .def("GetTransducer", &gz4d::Simrad::Client::GetTransducer,return_value_policy<copy_non_const_reference>())
            .def("GetBeamConfigurationManager", &gz4d::Simrad::Client::GetBeamConfigurationManager,return_value_policy<copy_non_const_reference>())
            .def("Create", &gz4d::Simrad::Client::Create)
            .staticmethod("Create")
            ;

}
