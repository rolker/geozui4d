#ifndef GZ4D_OGR_CURVE_H
#define GZ4D_OGR_CURVE_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/Geometry.h>

class OGRCurve;

namespace gz4d
{
    namespace ogr
    {
        class Curve: public Geometry
        {
            std::shared_ptr<OGRCurve> curve;
            public:
                Curve(Geometry const &g)
                :Geometry(g)
                {
                    curve = std::dynamic_pointer_cast<OGRCurve>(geometry);
                }

                operator bool() const {return bool(curve);}
        };
    }
}

#endif
