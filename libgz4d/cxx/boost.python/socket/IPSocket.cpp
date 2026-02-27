#include "../boost_python.h"
#include <gz4d/socket/IPSocket.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(IPSocket)
{
    class_<gz4d::IPSocket, bases<gz4d::Socket> >("IPSocket")
        .def("Interfaces", &gz4d::IPSocket::Interfaces)
        .staticmethod("Interfaces")
    ;
}

