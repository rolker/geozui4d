#include "../boost_python.h"
#include <gz4d/nav/Attitude.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Attitude)
{
    class_<gz4d::nav::Attitude<float>, bases<gz4d::nav::Heading<float> > >("Attitude")
            .def_readwrite("pitch", &gz4d::nav::Attitude<float>::pitch)
            .def_readwrite("roll", &gz4d::nav::Attitude<float>::roll)
            ;
}
