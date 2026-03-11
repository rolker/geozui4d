// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef OGR_POLYGON_H
#define OGR_POLYGON_H

#include"engine/SimpleObject.h"
#include "objects/ogr/OGR_Limits.h"
#include <gz4d/Point.h>
#include <gz4d/geo/Projection.h>
#include <vector>

class OGR_Polygon: public SimpleObject
{
    std::vector<gz4d::Point<float> > exteriorRing;
    bool exteriorRingDir;

    std::vector<std::vector<gz4d::Point<float> > > interiorRings;
    std::vector<bool> interiorRingDirs;

    public:
        OGR_Polygon(gz4d::ogr::Polygon p, gz4d::ogr::SpatialReference spatialRef, double xref, double yref, OGR_Limits const &limits, float minSpan);
        void Draw(float degradation, Window3D *win);
};

#endif
