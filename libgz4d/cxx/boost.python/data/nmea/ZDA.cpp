#define GZ4D_BOOST_PYTHON
#include <boost/python.hpp>
#include <gz4d/data/nmea/ZDA.h>

using namespace boost::python;


BOOST_PYTHON_MODULE(ZDA)
{
    class_<gz4d::nmea::ZDA, bases<gz4d::nmea::Time> >("ZDA", init<gz4d::nmea::Sentence const&>())
            .def(init<>())
            .def_readwrite("day", &gz4d::nmea::ZDA::day)
            .def_readwrite("month", &gz4d::nmea::ZDA::month)
            .def_readwrite("year", &gz4d::nmea::ZDA::year)
            ;
}
