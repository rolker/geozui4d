#include "../boost_python.h"
#include <gz4d/nav/Heading.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Heading)
{
    class_<gz4d::nav::Heading<float> >("Heading")
            .def_readwrite("heading", &gz4d::nav::Heading<float>::heading)
            ;
}
