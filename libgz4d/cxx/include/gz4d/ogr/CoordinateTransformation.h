#ifndef GZ4D_OGR_COORDINATETRANSFORMATION_H
#define GZ4D_OGR_COORDINATETRANSFORMATION_H

#include <gz4d/ogr/common.h>
#include <memory>
#include <gz4d/ogr/SpatialReference.h>

class OGRCoordinateTransformation;

namespace gz4d
{
    namespace ogr
    {

        class  CoordinateTransformation
        {
            std::shared_ptr<OGRCoordinateTransformation> ct;
            public:
                CoordinateTransformation(){}
                CoordinateTransformation(SpatialReference const &source, SpatialReference const &target)
                :ct(OGRCreateCoordinateTransformation(source.spatialReference.get(), target.spatialReference.get()))
                {
                }
                

                bool Transform(int nCount, double * x, double * y, double * z = NULL) const
                {
                    return ct->Transform(nCount,x,y,z);
                }
                
                operator bool() const {return bool(ct);}
        };
    }
}

#endif
