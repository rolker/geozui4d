#ifndef GZ4D_OGR_POINT_H
#define GZ4D_OGR_POINT_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/Geometry.h>
#include <gz4d/Point.h>
#include <ogr_geometry.h>

namespace gz4d
{
    namespace ogr
    {
        class LineString;
        class MultiPoint;

        class Point: public Geometry
        {
            friend class LineString;
            friend class MultiPoint;

            std::shared_ptr<OGRPoint> point;

            public:
                Point(std::shared_ptr<OGRMultiPoint> mp, int i);
                Point();
                Point(Geometry const &g);
                operator bool() const {return bool(point);}
                double getX() const;
                double getY() const;
                double getZ() const;
                operator gz4d::Point<double>() const {return gz4d::Point<double>(getX(),getY(),getZ());}
        };
    }
}

inline gz4d::ogr::Point::Point()
{
    point = std::shared_ptr<OGRPoint>(new OGRPoint);
    geometry = point;
}

inline gz4d::ogr::Point::Point(Geometry const &other)
:Geometry(other)
{
    point = std::dynamic_pointer_cast<OGRPoint>(geometry);
}

inline gz4d::ogr::Point::Point(std::shared_ptr<OGRMultiPoint> mp, int i)
:Geometry(mp->getGeometryRef(i),mp)
{
    point = std::dynamic_pointer_cast<OGRPoint>(geometry);
}

inline double gz4d::ogr::Point::getX() const
{
    return point->getX();
}

inline double gz4d::ogr::Point::getY() const
{
    return point->getY();
}

inline double gz4d::ogr::Point::getZ() const
{
    return point->getZ();
}


#endif
