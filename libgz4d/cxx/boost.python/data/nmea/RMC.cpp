#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/RMC.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(RMC)
{
    class_<gz4d::nmea::RMC, bases<gz4d::nmea::Time, gz4d::nmea::LatLon> >("RMC", init<gz4d::nmea::Sentence const&>())
            .def(init<>())
            .def_readwrite("status", &gz4d::nmea::RMC::status)
            .def_readwrite("sog", &gz4d::nmea::RMC::sog)
            .def_readwrite("track", &gz4d::nmea::RMC::track)
            .def_readwrite("day", &gz4d::nmea::RMC::day)
            .def_readwrite("month", &gz4d::nmea::RMC::month)
            .def_readwrite("year", &gz4d::nmea::RMC::year)
            .def_readwrite("magnetic_declination", &gz4d::nmea::RMC::magnetic_declination)
            ;
}
