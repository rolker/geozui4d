#include "../boost_python.h"
#include <gz4d/time/Duration.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Duration)
{
    class_<gz4d::time::Duration>("Duration",init<double>())
        .def(init<std::string const &>())
        .def("asSeconds",&gz4d::time::Duration::asSeconds)
        .def("__str__", &gz4d::time::Duration::operator std::string)
    ;
}
