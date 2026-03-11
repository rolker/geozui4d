// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef OGR_FEATURE_H
#define OGR_FEATURE_H

#include<gz4d/glo.h>
#include"engine/SimpleObject.h"
#include <memory>
#include "objects/ogr/OGR_Limits.h"

class OGR_Feature : public SimpleObject
{
    std::shared_ptr<SimpleObject> geometry;
    public:
        OGR_Feature(){}
        OGR_Feature(gz4d::ogr::Feature f, gz4d::ogr::SpatialReference sr, double xref, double yref, OGR_Limits const &limits, float minSpan = 0.0);
        void Draw(float degradation, Window3D *win);
        operator bool(){return bool(geometry);}
};

#endif
