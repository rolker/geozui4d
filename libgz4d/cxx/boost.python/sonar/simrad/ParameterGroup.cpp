// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>
#include <gz4d/sonar/Simrad/net/Parameter.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(ParameterGroup)
{
    class_<gz4d::Simrad::ParameterGroup, bases<gz4d::Producer<gz4d::Time> >, std::shared_ptr<gz4d::Simrad::ParameterGroup>, boost::noncopyable>("ParameterGroup", init<std::shared_ptr<gz4d::Simrad::ParameterManager> &>())
            .def("Add", &gz4d::Simrad::ParameterGroup::Add)
            .def("__getitem__", &gz4d::Simrad::ParameterGroup::operator[])
            ;

    class_<std::map<std::string,gz4d::Simrad::ParameterGroup::Ptr> >("ParameterGroupMap")
            .def(map_indexing_suite<std::map<std::string,gz4d::Simrad::ParameterGroup::Ptr>, true>())
            ;
}
