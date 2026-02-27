#ifndef GZ4D_OGR_MULTIPOINT_H
#define GZ4D_OGR_MULTIPOINT_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/GeometryCollection.h>
#include <boost/iterator/iterator_adaptor.hpp>
#include <gz4d/ogr/Point.h>
#include <ogr_geometry.h>

namespace gz4d
{
    namespace ogr
    {

        class MultiPoint: public DerivedGeometryCollection<Point,OGRMultiPoint>
        {
            public:
                inline MultiPoint(Geometry const &g);
        };
    }
}


inline gz4d::ogr::MultiPoint::MultiPoint(Geometry const &other)
:DerivedGeometryCollection<Point,OGRMultiPoint>(other)
{
    ogrObject = std::dynamic_pointer_cast<OGRMultiPoint>(geometry);
}


#endif
