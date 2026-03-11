// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_AXIS_H
#define GZ4D_GEO_AXIS_H

#include <gz4d/Angle.h>
#include <gz4d/crs/Axis.h>

namespace gz4d
{
    namespace crs
    {
        namespace dir 
        {
            /// Axis positive direction is north. In a geographic or projected
            /// CRS, north is defined through the geodetic datum. In an
            /// engineering CRS, north may be defined with respect to an
            /// engineering object rather than a geographical direction.
            struct North
            {
            };
            
            /// Axis positive direction is π/2 radians clockwise from north.
            struct East
            {
            };
            
            /// Axis positive direction is π radians clockwise from north.
            struct South
            {
            };
            
            /// Axis positive direction is 3π/2 radians clockwise from north.
            struct West
            {
            };
            
            /// Axis positive direction is in the equatorial plane from the
            /// centre of the modelled earth towards the intersection of the
            /// equator with the prime meridian.
            struct GeocentricX
            {
            };
            
            /// Axis positive direction is in the equatorial plane from the
            /// centre of the modelled earth towards the intersection of the
            /// equator and the meridian π/2 radians eastwards from the
            /// prime meridian.
            struct GeocentricY
            {
            };
            
            /// Axis positive direction is from the centre of the modelled
            /// earth parallel to its rotation axis and towards its north pole.
            struct GeocentricZ
            {
            };
            
            
        }
        
        namespace axis
        {
            
            enum AxisDirections
            {
            };
            
            
            template <typename B> struct Latitude: Axis<B, dir::North, Degree>
            {
                enum {latitude_index= Axis<B, dir::North, Degree>::index};
            };
            
            template <typename B> struct Longitude: Axis<B, dir::East, Degree>
            {
                enum {longitude_index= Axis<B, dir::East, Degree>::index};
            };
            
            template <typename B> struct LatLon: Longitude<Latitude<B> >
            {
            };
            
            template <typename B> struct LatLonHeight: Axis<LatLon<B>,dir::Up,Meter>
            {
                enum {ellipsoidal_height_index=Axis<LatLon<B>,dir::Up,Meter>::index};
            };
            
            template <typename B> struct GeoidalHeight: Axis<B,dir::Up,Meter>
            {
                enum {geoidal_height_index=Axis<B,dir::Up,Meter>::index};
            };
        }
    }
}

#endif

