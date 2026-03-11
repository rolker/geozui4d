// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_GEODETICPOINT_H
#define GZ4D_GEO_GEODETICPOINT_H

#include <gz4d/gz4d.h>
#include <gz4d/geo/Position.h>
#include <gz4d/Vector.h>
#include "ReferenceFrame.h"

namespace gz4d
{
    namespace geo
    {

        /// Geographic position expressed as latitude, longitude and height.
        /// A geodetic latitude is the angle between the normal of the surface
        /// of the ellipsoid and the plane of the equator.
        template<typename T> struct GeodeticPoint: public Position<T>
        {
            T altitude;
            GeodeticPoint():altitude(Nan<T>()){}
            GeodeticPoint(typename Position<T>::latitude_type lat, typename Position<T>::longitude_type lon, T a):Position<T>(lat,lon),altitude(a){}

            /// Construct from point with x=lon, y=lat, z=altitude
            explicit GeodeticPoint(Vector<T,3> const &p):Position<T>(p[1],p[0]),altitude(p[2]){}

            template<typename ET> operator Point<T,ReferenceFrame<Geodetic<LatLon>,ET> >() const
            {
                return Point<T,ReferenceFrame<Geodetic<LatLon>,ET> >(Position<T>::latitude.value(), Position<T>::longitude.value(),altitude);
            }
        };

    }
}

#endif
