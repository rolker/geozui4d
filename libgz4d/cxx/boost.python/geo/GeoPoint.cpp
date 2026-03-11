// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/geo/Point.h>
#include <gz4d/geo/WGS84.h>

using namespace boost::python;

template<typename T, typename RF, typename ORF> inline void WrapPoint(char const *name)
{
    class_<gz4d::geo::Point<T,RF>, bases<gz4d::Point<T> > >(name,init<>())
            .def(init<T,T,T>())
            .def(init<gz4d::Point<T> >())
            .def(init<gz4d::geo::Point<T,ORF> >())
            ;
}


BOOST_PYTHON_MODULE(GeoPoint)
{
    WrapPoint<double,gz4d::geo::WGS84::LatLon,gz4d::geo::WGS84::ECEF>("LatLonPoint");
    WrapPoint<double,gz4d::geo::WGS84::ECEF,gz4d::geo::WGS84::LatLon>("ECEFPoint");
}
