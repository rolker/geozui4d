// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_WGS84_H
#define GZ4D_GEO_WGS84_H

#include <gz4d/geo.h>
#include <gz4d/geo/Ellipsoid.h>
#include <gz4d/geo/ReferenceFrame.h>

namespace gz4d
{
    namespace geo
    {
        namespace WGS84
        {
            struct EllipsoidSpecs
            {
                static double a() {return 6378137.0;}
                static double b() {return 6356752.3142;}
                static double w() {return 7292115e-11;}
                static double e2() { return 1.0-( 6356752.3142* 6356752.3142)/(6378137.0*6378137.0);}
            };

            typedef geo::Ellipsoid<EllipsoidSpecs> Ellipsoid;

//             class Ellipsoid
//             {
//                 static geo::Ellipsoid e;
// 
//                 public:
//                     geo::Ellipsoid const &operator()() const {return e;}
//             };

            typedef ReferenceFrame<Geodetic<LatLon>, Ellipsoid> LatLon;
            typedef ReferenceFrame<Geodetic<LonLat>, Ellipsoid> LonLat;
            typedef ReferenceFrame<ECEF<>, Ellipsoid> ECEF;
        }
    }
}

//gz4d::geo::Ellipsoid gz4d::geo::WGS84::Ellipsoid::e(6378137.0, 6356752.3142 , 7292115e-11);

#endif
