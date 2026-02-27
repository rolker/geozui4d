#include "../boost_python.h"
#include <gz4d/geo/LocalENU.h>

using namespace boost::python;

gz4d::Point<double> (gz4d::geo::LocalENU<>::*call_point)(gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF> const&) const = &gz4d::geo::LocalENU<>::toLocal;
std::vector<gz4d::Point<double> > (gz4d::geo::LocalENU<>::*call_vector)(std::vector<gz4d::geo::Point<double,gz4d::geo::WGS84::ECEF> > const &) const = &gz4d::geo::LocalENU<>::toLocal;

BOOST_PYTHON_MODULE(LocalENU)
{
    class_<gz4d::geo::LocalENU<> >("LocalENU")
            .def(init<gz4d::geo::Point<double,gz4d::geo::WGS84::LatLon> >())
            .def("toECEF",&gz4d::geo::LocalENU<>::toECEF)
            .def("__call__",call_point)
            .def("__call__",call_vector)
            ;
}
