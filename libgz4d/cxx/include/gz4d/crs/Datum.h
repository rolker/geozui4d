// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CRS_DATUM_H
#define GZ4D_CRS_DATUM_H


namespace gz4d
{
    namespace crs
    {
        namespace datum
        {
            
            struct NullAnchor
            {
            };
            
            struct NullEpoch
            {
            };
            
            /// A datum specifies the relationship of a coordinate system to the earth, thus ensuring that the abstract
            /// mathematical concept “coordinate system” can be applied to the practical problem of describing positions of
            /// features on or near the earth’s surface by means of coordinates.
            template <typename ANCHOR, typename EPOCH> struct Datum
            {
            };
            
            /// An engineering datum defines the origin of an engineering coordinate reference system, and is used in a region around
            /// that origin. This origin can be fixed with respect to the earth (such as a defined point at a construction site), or be a
            /// defined point on a moving vehicle (such as on a ship or satellite).
            template <typename ANCHOR, typename EPOCH> struct EngineeringDatum: Datum<ANCHOR, EPOCH>
            {
            };
            
            struct UnspecifiedEngineeringDatum: EngineeringDatum<NullAnchor,NullEpoch>
            {
            };
            
            /// Anchor type used to define an Engineering Datum used in expressing coordinates relative
            /// to a platform's master reference plane. For example, a ship's coordinate system origin used by 
            /// a motion sensor.
            struct MasterReferencePlane
            {
            };
            
            /// An image datum defines the origin of an image coordinate reference system, and is used in a local context only. For an
            /// image datum, the anchor point is usually either the centre of the image or the corner of the image.
            template <typename ANCHOR, typename EPOCH> struct ImageDatum: Datum<ANCHOR, EPOCH>
            {
            };
            
            
            enum TemporalOrigins
            {
                UnixEpoch
            };
            
            template <int T> struct TemporalOrigin
            {
            };
            
            template <typename ORIGIN, typename ANCHOR, typename EPOCH> struct TemporalDatum: Datum<ANCHOR, EPOCH>
            {
            };
            
        }        
    }
}

#endif
