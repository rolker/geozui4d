// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/sonar/Simrad/net/ServerManager.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(ServerManager)
{
    gz4d::Simrad::Server (gz4d::Simrad::ServerManager::*GetS)(std::string) = &gz4d::Simrad::ServerManager::Get;

    gz4d::Simrad::Server (gz4d::Simrad::ServerManager::*GetI)(int) = &gz4d::Simrad::ServerManager::Get;

    gz4d::Simrad::Server (gz4d::Simrad::ServerManager::*GetX)(boost::property_tree::ptree const &) = &gz4d::Simrad::ServerManager::Get;

    class_<gz4d::Simrad::ServerManager>("ServerManager")
            .def(init<optional<uint16_t, std::string, std::string> >())
            .def("GetList", &gz4d::Simrad::ServerManager::GetList)
            .def("Get",GetS)
            .def("Get",GetI)
            .def("Get",GetX)
            ;
}
