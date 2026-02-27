#ifndef GZ4D_OGR_GEOMETRY_H
#define GZ4D_OGR_GEOMETRY_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <ogr_geometry.h>
#include<ogr_feature.h>
#include <gz4d/ogr/SpatialReference.h>
#include <gz4d/ogr/Envelope.h>


namespace gz4d
{
    namespace ogr
    {

        class Feature;
        class Polygon;
        class GeometryCollection;
        class Point;

        class Geometry
        {
            friend class Feature;
            friend class Polygon;
            friend class GeometryCollection;
            friend class Point;

            protected:
                std::shared_ptr<OGRGeometry> geometry;
            private:
                std::shared_ptr<void> owner;
                Geometry(OGRGeometry *g, std::shared_ptr<void> o);
                Geometry(std::shared_ptr<OGRGeometryCollection> gc, int i);

            public:
                Geometry();
                Geometry(Geometry const &other);
                operator bool() const {return bool(geometry);}
                SpatialReference getSpatialReference();
                Envelope getEnvelope();
                std::string getName(){return geometry->getGeometryName();}
        };
    }
}

inline gz4d::ogr::Geometry::Geometry()
{
}

inline gz4d::ogr::Geometry::Geometry(OGRGeometry *g, std::shared_ptr<void> o)
:geometry(std::shared_ptr<OGRGeometry>(g,NullDeleter())),owner(o)
{
}

inline gz4d::ogr::Geometry::Geometry(std::shared_ptr<OGRGeometryCollection> gc, int i)
:geometry(std::shared_ptr<OGRGeometry>(gc->getGeometryRef(i),NullDeleter())),owner(gc)
{
}

inline gz4d::ogr::Geometry::Geometry(Geometry const &other)
:geometry(other.geometry),owner(other.owner)
{
}

inline gz4d::ogr::SpatialReference gz4d::ogr::Geometry::getSpatialReference()
{
    return SpatialReference(geometry->getSpatialReference());
}

inline gz4d::ogr::Envelope gz4d::ogr::Geometry::getEnvelope()
{
    
    Envelope ret;
    geometry->getEnvelope(ret.envelope.get());
    return ret;
}


#endif
