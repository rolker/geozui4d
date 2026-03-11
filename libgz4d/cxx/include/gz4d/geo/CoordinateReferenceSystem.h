// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_COORDINATEREFERENCESYSTEM_H
#define GZ4D_GEO_COORDINATEREFERENCESYSTEM_H

#include <gz4d/crs/CoordinateReferenceSystem.h>

namespace gz4d
{
    namespace crs
    {
        /// Type of coordinate reference system that deals with the earth’s curvature by taking the 3D
        /// spatial view, which obviates the need to model the earth’s curvature. The origin of a geocentric CRS is at
        /// the approximate centre of mass of the earth.
        template <typename CS, typename DATUM> struct Geocentric: CoordinateReferenceSystem<CS, DATUM>
        {
        };
        
        /// Type of coordinate reference system based on an ellipsoidal approximation of the geoid.
        /// This provides an accurate representation of the geometry of geographic features for a large portion of the
        /// earth’s surface. Geographic coordinate reference systems can be 2D or 3D. A 2D Geographic CRS is
        /// used when positions of features are described on the surface of the ellipsoid; a 3D Geographic CRS is
        /// used when positions are described on, above or below the ellipsoid.
        template <typename CS, typename DATUM> struct Geographic: CoordinateReferenceSystem<CS, DATUM>
        {
        };
        

        /// Type of coordinate reference system that is based on an approximation of the shape of the
        /// earth’s surface by a plane. The distortion that is inherent to the approximation is carefully controlled and
        /// known. Distortion correction is commonly applied to calculated bearings and distances to produce values
        /// that are a close match to actual field values.
        template <typename BASE, typename CS> struct Projected: DerivedCoordinateReferenceSystem<BASE, CS>
        {
        };

        /// Type of coordinate reference system used for the recording of heights or depths. Vertical CRSs
        /// make use of the direction of gravity to define the concept of height or depth, but its relationship with
        /// gravity may not be straightforward. By implication therefore, ellipsoidal heights (h) cannot be captured in a
        /// vertical coordinate reference system: ellipsoidal heights cannot exist independently, but only as an
        /// inseparable part of a 3D coordinate tuple defined in a geographic 3D coordinate reference system.
        template <typename CS, typename DATUM> struct Vertical: CoordinateReferenceSystem<CS, DATUM>
        {
        };
    }
}

#endif
