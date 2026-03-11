// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_CRS_COORDINATEREFERENCESYSTEM_H
#define GZ4D_CRS_COORDINATEREFERENCESYSTEM_H


namespace gz4d
{
    namespace crs
    {
        /// Loosely based on ISO 19111
            
        /// A coordinate reference system consists of one coordinate system that is related to the earth through one
        /// datum.
        template <typename CS, typename DATUM> struct CoordinateReferenceSystem: CS
        {
            typedef DATUM datum_type;
        };
            
        /// Type of coordinate reference system that is used only in a contextually local sense. This
        /// sub-type is used to model two broad categories of local coordinate reference systems:
        /// - earth-fixed systems, applied to engineering activities on or near the surface of the earth;
        /// - coordinates on moving platforms such as road vehicles, vessels, aircraft or spacecraft.
        template <typename CS, typename DATUM> struct Engineering: CoordinateReferenceSystem<CS, DATUM>
        {
        };
            
        /// An Image CRS is an Engineering CRS applied to images. Image CRSs are treated as a separate
        /// sub-type because for images a separate user community with its own vocabulary exists. The definition of
        /// the associated Image Datum contains two data attributes not relevant for other datums and coordinate
        /// reference systems.
        //             template <typename B, template <typename> class CS, typename DATUM> struct ImageCRS: CoordinateReferenceSystem<B, Type<Image>, CS, DATUM>
        //             {
            //             };
        
        
        /// Used for the recording of time in association with any of the listed spatial coordinate reference
        /// systems.
        //             template <typename B, template <typename> class CS, typename DATUM> struct TemporalCRS: CoordinateReferenceSystem<B, Type<Temporal>, CS, DATUM>
        //             {
            //             };
            
        /// The traditional separation of horizontal and vertical position described in Clause 6 has resulted in coordinate
        /// reference systems that are horizontal (2D) and vertical (1D) in nature, as opposed to truly 3-dimensional. It is
        /// established practice to combine the horizontal coordinates of a point with a height or depth from a different
        /// coordinate reference system.
        template <typename CRF1, template <typename> class CS, typename DATUM> struct CompoundCoordinateReferenceSystem:  CRF1,CS<typename CRF1::last_axis_type>
        {
            enum { dimensions=CS<typename CRF1::last_axis_type>::index+1 };
        };
            
        /// Some coordinate reference systems are defined by applying a coordinate conversion to another coordinate
        /// reference system. Such a coordinate reference system is called a Derived CRS and the coordinate reference
        /// system from which it was derived is called the Base CRS. The best-known example of a Derived CRS is a
        /// Projected CRS, which is always derived from a base Geographic CRS by applying the coordinate conversion
        /// known as a map projection.
        template <typename BASE, typename CS> struct DerivedCoordinateReferenceSystem: CoordinateReferenceSystem<CS, typename BASE::datum_type>
        {
            typedef BASE base_crs;
        };
    }
}

#endif
