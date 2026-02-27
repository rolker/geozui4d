#ifndef GZ4D_IMAGE_INFO_H
#define GZ4D_IMAGE_INFO_H

#include <gz4d/gz4d.h>
#include <string>

namespace gz4d
{
    namespace image
    {
        struct Info
        {
            std::size_t width;
            std::size_t height;
            std::string format; ///< Combination of R,G,B,A,L and optional sizes. ex: RGB, RGBA8, L16A8
            Info():width(0),height(0){}
            Info(std::size_t w, std::size_t h, std::string const &f):width(w),height(h),format(f){}
        };
    }
}

#endif
