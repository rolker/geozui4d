// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CRS_COORDINATESYSTEM_H
#define GZ4D_CRS_COORDINATESYSTEM_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    namespace crs
    {
        namespace cs
        {
            /// Component of a CoordinateReferenceSystem.
            /// The coordinate system is composed of a set of coordinate axes with specified units of measure. This concept
            /// implies the mathematical rules that define how coordinate values are calculated from distances, angles and
            /// other geometric elements and vice versa.
            template <typename A> struct CoordinateSystem:  A
            {
                typedef A last_axis_type;
                enum {dimensions=A::index+1};
            };
            
            
            /// A two- or three-dimensional coordinate system with straight axes that are not necessarily orthogonal.
            template <typename A> struct Affine: CoordinateSystem<A>
            {
            };
            
            /// A three-dimensional coordinate system with one distance measured from the origin and two angular coordinates. Not to be
            /// confused with an ellipsoidal coordinate system based on an ellipsoid "degenerated" into a sphere.
            template <typename A> struct Spherical: CoordinateSystem<A>
            {
            };
            
            /// A three-dimensional coordinate system consisting of a polar coordinate system extended by a straight coordinate axis
            /// perpendicular to the plane spanned by the polar coordinate system.
            template <typename A> struct Cylindrical: CoordinateSystem<A>
            {
            };
            
            /// A two-dimensional coordinate system in which position is specified by the distance from the origin and the angle between
            /// the line from the origin to a point and a reference direction.
            template <typename A> struct Polar: CoordinateSystem<A>
            {
            };
            
            /// A one-dimensional coordinate system that consists of the points that lie on the single axis described. The associated
            /// coordinate is the distance – with or without offset – from the specified datum to the point along the axis. Example: usage of
            /// the line feature representing a pipeline to describe points on or along that pipeline.
            template <typename A> struct Linear: CoordinateSystem<A>
            {
            };
            
            /// A one-dimensional coordinate system containing a time axis, used to describe the temporal position of a point in the
            /// specified time units from a specified time origin. 
            template <typename A> struct Time: CoordinateSystem<A>
            {
            };
            
            /// A two- or three-dimensional coordinate system that consists of any combination of coordinate axes not covered by any other
            /// Coordinate System type. An example is a multilinear coordinate system which contains one coordinate axis that may have
            /// any 1-D shape which has no intersections with itself. This non-straight axis is supplemented by one or two straight axes to
            /// complete a 2 or 3 dimensional coordinate system. The non-straight axis is typically incrementally straight or curved.
            template <typename A> struct UserDefined: CoordinateSystem<A>
            {
            };
            
        }
    }
}

#endif
