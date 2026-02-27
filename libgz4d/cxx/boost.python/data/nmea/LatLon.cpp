#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/LatLon.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(LatLon)
{
    class_<gz4d::nmea::LatLon>("LatLon")
            .def_readwrite("latitude", &gz4d::nmea::LatLon::latitude)
            .def_readwrite("longitude", &gz4d::nmea::LatLon::longitude)
            ;
}
