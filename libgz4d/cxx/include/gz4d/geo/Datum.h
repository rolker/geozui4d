#ifndef GZ4D_GEO_DATUM_H
#define GZ4D_GEO_DATUM_H

#include <gz4d/crs/Datum.h>

namespace gz4d
{
    namespace crs
    {
        namespace datum
        {
            /// A geodetic datum defines the location and precise orientation in 3-dimensional space of a defined ellipsoid (or sphere)
            /// that approximates the shape of the earth, or of a Cartesian coordinate system centered in this ellipsoid (or sphere).
            template <typename ELLIPSOID, typename ANCHOR=NullAnchor, typename EPOCH=NullEpoch> struct GeodeticDatum: Datum<ANCHOR, EPOCH>
            {
            };
            
            enum VerticalDatumTypes
            {
                /// The zero value of the associated vertical
                /// coordinate system axis is defined to
                /// approximate a constant potential surface,
                /// usually the geoid. Such a reference surface
                /// is usually determined by a national or
                /// scientific authority, and is then a well-
                /// known, named datum.
                Geoidal,
                /// The zero point of the vertical axis is defined
                /// by a surface that has meaning for the
                /// purpose which the associated vertical
                /// measurements are used for. For hydrographic charts, this is often a predicted
                /// nominal sea surface (i.e., without waves or other wind and current effects) that occurs
                /// at low tide.
                Depth,
                /// Atmospheric pressure is the basis for the
                /// definition of the origin of the associated
                /// vertical coordinate system axis.
                Barometric,
                /// In some cases, e.g. oil exploration and
                /// production, a geological feature, such as the
                /// top or bottom of a geologically identifiable
                /// and meaningful subsurface layer, is used as
                /// a vertical datum. Other variations to the
                /// above three vertical datum types may exist
                /// and are all included in this type.
                OtherSurface
            };
            
            template <int T> struct VerticalDatumType
            {
            };
            
            /// A textual description and/or a set of parameters identifying a particular reference level surface used as a zero-height or
            /// zero-depth surface, including its position with respect to the Earth. There are several types of vertical datum, and each
            /// may place constraints on the coordinate axis with which it is combined to create a Vertical CRS.
            template <typename VDT, typename ANCHOR=NullAnchor, typename EPOCH=NullEpoch> struct VerticalDatum: Datum<ANCHOR, EPOCH>
            {
            };
            
            template <typename GEOID, typename ANCHOR=NullAnchor, typename EPOCH=NullEpoch> struct GeoidalVerticalDatum: VerticalDatum<VerticalDatumType<Geoidal>,ANCHOR,EPOCH>
            {
            };
            
        }        
    }
}

#endif
