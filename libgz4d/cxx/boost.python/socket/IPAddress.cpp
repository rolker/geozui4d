#include "../boost_python.h"
#include <gz4d/socket/IPSocket.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(IPAddress)
{

    class_<gz4d::IPSocket::Address>("IPAddress")
            .def(init<optional<std::string, uint16_t> >())
            .def("GetAddress", &gz4d::IPSocket::Address::GetAddress)
            .def("GetPort", &gz4d::IPSocket::Address::GetPort)
            ;

}
