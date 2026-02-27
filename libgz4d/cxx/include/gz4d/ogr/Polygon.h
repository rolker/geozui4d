#ifndef GZ4D_OGR_POLYGON_H
#define GZ4D_OGR_POLYGON_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/Geometry.h>
#include <gz4d/data/Polygon.h>
#include <gz4d/Point.h>
#include <gz4d/ogr/LinearRing.h>
#include <ogr_geometry.h>

namespace gz4d
{
    namespace ogr
    {
        class Polygon: public Geometry
        {
            std::shared_ptr<OGRPolygon> polygon;
            public:
                Polygon(Geometry const &g);
                operator bool() const {return bool(polygon);}
                LinearRing getExteriorRing();
                LinearRing getInteriorRing(int n);
                int getNumInteriorRings();
                operator gz4d::Polygon<gz4d::Point<double> >();
        };
    }
}

inline gz4d::ogr::Polygon::Polygon(Geometry const &other)
:Geometry(other)
{
    polygon = std::dynamic_pointer_cast<OGRPolygon>(geometry);
}

inline gz4d::ogr::LinearRing gz4d::ogr::Polygon::getExteriorRing()
{
    return Geometry(polygon->getExteriorRing(),owner);
}

inline gz4d::ogr::LinearRing gz4d::ogr::Polygon::getInteriorRing(int n)
{
    return Geometry(polygon->getInteriorRing(n),owner);
}

inline int gz4d::ogr::Polygon::getNumInteriorRings()
{
    return polygon->getNumInteriorRings();
}

inline gz4d::ogr::Polygon::operator gz4d::Polygon<gz4d::Point<double> >()
{
    gz4d::Polygon<gz4d::Point<double> > ret;
    LinearRing ring = getExteriorRing();
    if(ring)
    {
        if(ring.isClockwise())
            ret.insert(ring.rbegin(),ring.rend());
        else
            ret.insert(ring.begin(),ring.end());
    }
    
    for(unsigned int i=0; i < getNumInteriorRings(); ++i)
    {
        ring = getInteriorRing(i);
        if(ring)
        {
            if(ring.isClockwise())
                ret.insert(i,ring.rbegin(),ring.rend());
            else
                ret.insert(i,ring.begin(),ring.end());
        }
    }
    return ret;
}

#endif
