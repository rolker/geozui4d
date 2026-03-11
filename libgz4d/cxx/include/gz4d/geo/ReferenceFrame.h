// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_REFERENCEFRAME_H
#define GZ4D_GEO_REFERENCEFRAME_H

#include <gz4d/gz4d.h>
#include <gz4d/geo/CoordinateFrame.h>
#include <gz4d/geo/Point.h>

namespace gz4d
{
    namespace geo
    {
        template <typename CT, typename ET> struct ReferenceFrame
        {
            typedef CT coordinate_type;
            typedef ET ellipsoid_type;
        };


        template <typename CF> struct ECEF;

        template <typename CF=LatLon> struct Geodetic
        {
            typedef CF coordinate_format;

            template <typename T, typename OCF, typename ET> Point<T, ReferenceFrame<Geodetic<CF>,ET> > operator()(Point<T, ReferenceFrame<Geodetic<OCF>,ET> > const &p)
            {
                Point<T, ReferenceFrame<Geodetic<CF>,ET> > ret;
                ret[CF::Latitude] = p[OCF::Latitude];
                ret[CF::Longitude] = p[OCF::Longitude];
                ret[CF::Height] = p[OCF::Height];
            }

            template <typename T, typename OCF, typename ET> Point<T, ReferenceFrame<Geodetic<CF>,ET> > operator()(Point<T, ReferenceFrame<ECEF<OCF>,ET> > const &p) const
            {
                return ET::ToGeodetic(p);
            }

        };

        template <typename CF=XYZ> struct ECEF
        {
            typedef CF coordinate_format;

            template <typename T, typename OCF, typename ET> Point<T, ReferenceFrame<ECEF<CF>,ET> > operator()(Point<T, ReferenceFrame<Geodetic<OCF>,ET> > const &p)
            {
                return ET::ToEarthCenteredEarthFixed(p);
            }
        };
    }
}

#endif
