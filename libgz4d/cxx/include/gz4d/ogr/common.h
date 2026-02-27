#ifndef GZ4D_OGR_COMMON_H
#define GZ4D_OGR_COMMON_H

#include <gz4d/gz4d.h>
#include <memory>
#include <ogr_spatialref.h>

//gz4d_cmake depends GDAL


namespace gz4d
{
    namespace ogr
    {
        template<typename T> struct ogr_deleter
        {
            void operator()(T * p)
            {
                if(p) p->Dereference();
            }
        };

        template<typename T> std::shared_ptr<T> make_shared_from_ogr(T * p)
        {
            if(p)
                p->Reference();
            std::shared_ptr<T> ret(p,ogr_deleter<T>());
            return ret;
        }
    }
}

#endif
