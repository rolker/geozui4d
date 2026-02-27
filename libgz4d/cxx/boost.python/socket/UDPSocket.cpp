#include "../boost_python.h"
#include <gz4d/socket/UDPSocket.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(UDPSocket)
{
    class_<gz4d::UDPSocket, bases<gz4d::IPSocket> >("UDPSocket")
            ;
}
