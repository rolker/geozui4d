// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/socket/Socket.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Socket)
{
//     void (gz4d::Socket::*SendP)(gz4d::binary::Packet const *) = &gz4d::Socket::Send;
//     void (gz4d::Socket::*SendToP)(gz4d::binary::Packet const *, gz4d::Socket::Address const &) = &gz4d::Socket::SendTo;


    class_<gz4d::Socket>("Socket")
/*            .def("Bind", &gz4d::Socket::Bind)
            .def("Connect", &gz4d::Socket::Connect)
            .def("Send", SendP)
            .def("SendTo", SendToP)*/
            ;

}
