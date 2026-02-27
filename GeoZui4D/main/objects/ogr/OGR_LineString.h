#ifndef OGR_LINESTRING_H
#define OGR_LINESTRING_H

#include"engine/SimpleObject.h"
#include "objects/ogr/OGR_Limits.h"
#include <gz4d/ogr/LineString.h>
#include <gz4d/Point.h>
#include <gz4d/geo/Projection.h>
#include <vector>

class OGR_LineString: public SimpleObject
{
    std::vector<gz4d::Point<float> > points;

    public:
        OGR_LineString(gz4d::ogr::LineString p, gz4d::ogr::SpatialReference spatialRef, double xref, double yref, OGR_Limits const &limits, float minSpan);
        void Draw(float degradation, Window3D *win);
};

#endif
