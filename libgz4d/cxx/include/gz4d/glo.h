// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_H
#define GZ4D_GLO_H

#include <gz4d/gz4d.h>
#if defined(GZ4D_HEADLESS)
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
#else
  #ifdef GZ4D_GLEW
    #define GLEW_STATIC
    #include <GL/glew.h>
  #else
	#ifdef BOOST_MSVC
		#include <windows.h>
	#endif
	#include <GL/gl.h>
  #endif
#endif


#include <string>

namespace gz4d
{
    namespace glo
    {
        template <typename T> std::pair<GLenum,GLenum> PixelFormat()
        {
            std::pair<GLenum,GLenum> ret(0,0);

            std::string f = T().String();
            if (f == "RGB8" || f == "RGB16")
                ret.first = GL_RGB;
            else if(f == "RGBA8" || f == "RGBA16")
                ret.first = GL_RGBA;
            else if(f == "L8" || f == "L16")
                ret.first = GL_LUMINANCE;
            else if(f == "LA8" || f == "LA16")
                ret.first = GL_LUMINANCE_ALPHA;

            if(T().ChannelSize() == 1)
                ret.second = GL_UNSIGNED_BYTE;
            else
                ret.second = GL_UNSIGNED_SHORT;
            return ret;
        }

        template <typename T> GLenum Type(T dummy=0);

        template <> inline GLenum Type(double dummy){return GL_DOUBLE;}
        template <> inline GLenum Type(float dummy){return GL_FLOAT;}
        template <> inline GLenum Type(int dummy){return GL_INT;}
        template <> inline GLenum Type(unsigned int dummy){return GL_UNSIGNED_INT;}
        template <> inline GLenum Type(short dummy){return GL_SHORT;}
        template <> inline GLenum Type(unsigned short dummy){return GL_UNSIGNED_SHORT;}
        template <> inline GLenum Type(char dummy){return GL_BYTE;}
        template <> inline GLenum Type(unsigned char dummy){return GL_UNSIGNED_BYTE;}
    }
}

#endif

