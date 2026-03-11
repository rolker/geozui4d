// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_READPIXELS_H
#define GZ4D_GLO_READPIXELS_H

#include <gz4d/glo.h>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template <typename T> inline void ReadPixels(T const &raster,int x=0, int y=0)
        {
            std::pair<GLenum,GLenum> format = PixelFormat<typename T::value_type::pixel_format>();

            glReadPixels(x,y,raster.GetColumns(),raster.GetRows(),format.first,format.second,raster.raw());
        }

/*        inline void ReadPixels(Raster<Pixel<3> >::Ptr &raster,int x=0, int y=0)
        {
            glReadPixels(x,y,raster->GetColumns(),raster->GetRows(),GL_RGB,GL_UNSIGNED_BYTE,raster->Data());
        }*/
    }
}

#endif
