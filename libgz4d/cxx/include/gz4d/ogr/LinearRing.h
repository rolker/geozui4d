#ifndef GZ4D_OGR_LINEARRING_H
#define GZ4D_OGR_LINEARRING_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/LineString.h>
#include <ogr_geometry.h>

namespace gz4d
{
    namespace ogr
    {
        class LinearRing: public LineString
        {
            std::shared_ptr<OGRLinearRing> linearRing;
            public:
                LinearRing(Geometry const &g);
                operator bool() const {return bool(linearRing);}
                bool isClockwise() const;

        };
    }
}


inline gz4d::ogr::LinearRing::LinearRing(Geometry const &other)
:LineString(other)
{
    linearRing = std::dynamic_pointer_cast<OGRLinearRing>(geometry);
}

inline bool gz4d::ogr::LinearRing::isClockwise() const
{
    return linearRing->isClockwise();
}


#endif
