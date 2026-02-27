#ifndef GZ4D_GEO_COORDINATESYSTEM_H
#define GZ4D_GEO_COORDINATESYSTEM_H

#include <gz4d/crs/CoordinateSystem.h>
#include <gz4d/crs/Cartesian.h>
#include "Axis.h"

namespace gz4d
{
    namespace crs
    {
        namespace cs
        {
            /// A two- or three-dimensional coordinate system in which position is specified by geodetic latitude, geodetic longitude, and (in
            /// the three-dimensional case) ellipsoidal height.
            template <typename A> struct Ellipsoidal: CoordinateSystem<A>
            {
            };

            /// A one-dimensional coordinate system used to record the heights or depths of points. Such a coordinate system is usually
            /// dependent on the Earth's gravity field, perhaps loosely as when atmospheric pressure is the basis for the vertical coordinate
            /// system axis.
            template <typename A> struct Vertical: CoordinateSystem<A>
            {
            };
            
            
            template <typename B= crs::axis::InitAxis> struct GeoidalHeight: Vertical<axis::GeoidalHeight<B> >
            {
            };
            
            template <typename B=crs::axis::InitAxis> struct ECEF: Cartesian<axis::XYZ<B, dir::GeocentricX, dir::GeocentricY, dir::GeocentricZ,axis::Meter > >
            {
            };
            
            template <typename B=crs::axis::InitAxis> struct LatLon2D: Ellipsoidal< axis::LatLon<B> >
            {
            };
            
            template <typename B=crs::axis::InitAxis> struct LatLonHeight: Ellipsoidal< axis::LatLonHeight<B> >
            {
            };
            
        }
    }
}

#endif
