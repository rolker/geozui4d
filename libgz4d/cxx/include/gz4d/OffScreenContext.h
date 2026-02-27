#ifndef GZ4D_OFFSCREENCONTEXT_H
#define GZ4D_OFFSCREENCONTEXT_H

//gz4d_cmake depends osmesa

#include <gz4d/gz4d.h>

#include <gz4d/data/RasterContainer.h>
#include <gz4d/Pixel.h>

#include <gz4d/Exception.h>

#ifndef GLAPI
#define GLAPI
#endif
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#include <GL/osmesa.h>

namespace gz4d
{
    /// OpenGL context that renders to memory.
    /// Does not depend on a physical display
    /// or windowing library. (glx, wgl)
    class OffScreenContext: public RasterContainer<Pixel<RGB8> >
    {
        std::shared_ptr<osmesa_context> context;

        public:
            OffScreenContext(){}
            OffScreenContext(Raster<Pixel<RGB8> >::Ptr &r):RasterContainer<Pixel<RGB8> >(r){}

            /// Makes the OpenGL context current.
            void MakeCurrent()
            {
                if(!context)
                {
                    context = std::shared_ptr<osmesa_context>(OSMesaCreateContextExt( OSMESA_RGB, 16, 0, 0, NULL ),OSMesaDestroyContext);
                    if(!context)
                        throw Exception("Context creation problem\n");
                }
                if(!OSMesaMakeCurrent(context.get(), raster->raw(), GL_UNSIGNED_BYTE, raster->GetColumns() , raster->GetRows()))
                    throw Exception("Problem making context current");
            }
    };
};

#endif
