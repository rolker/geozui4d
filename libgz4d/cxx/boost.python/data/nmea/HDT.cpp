#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/HDT.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(HDT)
{
    class_<gz4d::nmea::HDT >("HDT", init<gz4d::nmea::Sentence const&>())
            .def(init<>())
            .def_readwrite("heading", &gz4d::nmea::HDT::heading)
            ;
}
