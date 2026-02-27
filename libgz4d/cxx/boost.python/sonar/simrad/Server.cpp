#include "../../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/sonar/Simrad/net/Server.h>
#include <gz4d/sonar/Simrad/net/Client.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Server)
{
    class_<gz4d::Simrad::Server>("Server")
            .def("GetAddress", &gz4d::Simrad::Server::GetAddress, return_internal_reference<>())
            .def("GetID", &gz4d::Simrad::Server::GetID)
            .def("GetType", &gz4d::Simrad::Server::GetType)
            .def("__str__", &gz4d::Simrad::Server::String)
            .def(!self)
            ;

    class_<gz4d::Simrad::Server::List>("ServerList")
            .def(vector_indexing_suite<gz4d::Simrad::Server::List>())
            ;
}
