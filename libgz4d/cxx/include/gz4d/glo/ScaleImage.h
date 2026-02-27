#ifndef GZ4D_GLO_SCALEIMAGE_H
#define GZ4D_GLO_SCALEIMAGE_H

#include <gz4d/glo.h>
#include <GL/glu.h>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include "PushAttrib.h"

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template<typename T> inline std::shared_ptr<Raster<T> > ScaleImage(std::shared_ptr<Raster<T> > raster, std::size_t newWidth, std::size_t newHeight)
        {
            std::shared_ptr<Raster<T> > newData(new Raster<T>);
            if(raster->empty())
                return newData;
            std::size_t rWidth = raster->GetColumns();
            std::size_t rHeight = raster->GetRows();
            if(newWidth == 0 && newHeight == 0)
            {
                newWidth = rWidth;
                newHeight = rHeight;
            }
            if(newWidth == 0)
                newWidth = (newHeight*rWidth)/rHeight;
            if(newHeight == 0)
                newHeight = (newWidth*rHeight)/rWidth;
            newData->Resize(newWidth,newHeight);
            std::pair<GLenum,GLenum> format = PixelFormat<typename T::pixel_format>();
            PushAttrib attrib;
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glPixelStorei(GL_PACK_ALIGNMENT,1);
            if(gluScaleImage(format.first,rWidth,rHeight,format.second,raster->raw(),newWidth,newHeight,format.second,newData->raw()))
                throw Exception("Error scaling image");
            return newData;
        }
    }
}

#endif
