// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RASTERCONTAINER_H
#define GZ4D_RASTERCONTAINER_H

#include <gz4d/gz4d.h>
#include <gz4d/data/Raster.h>

namespace gz4d
{
    /// \ingroup container
    template <typename T> class RasterContainer
    {
        public:
            RasterContainer():raster(new Raster<T>){}
            RasterContainer(typename Raster<T>::Ptr &r):raster(r){}
            RasterContainer(RasterContainer const &o):raster(o.raster){}
            template <typename OT> RasterContainer(RasterContainer<OT> &o)
            :raster(new Raster<T>(*(o.GetRaster()))){}

            void SetRaster(typename Raster<T>::Ptr &r) {raster = r;}
            typename Raster<T>::Ptr &GetRaster(){return raster;}
            operator Raster<T> const &(){return *raster;}
        protected:
            typename Raster<T>::Ptr raster;
    };
}

#endif
