// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_TEXTURE_H
#define GZ4D_GLO_TEXTURE_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>
#include <gz4d/glo/Object.h>
#include <gz4d/glo/Target.h>
#include <gz4d/glo/InternalFormat.h>
#include <gz4d/glo/PushClientAttrib.h>

#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/glo/Error.h>

namespace gz4d
{
    namespace glo
    {

        /// Manages an OpenGL texture map.
        /// \ingroup glo
        class Texture: public Target, public InternalFormat, public Object
        {

            static GLuint creator()
            {
                GLuint ret;
                glGenTextures(1,&ret);
                return ret;
            }
            
            static void deleter(GLuint i)
            {
                glDeleteTextures(1,&i);
            }
            
            public:
                Texture(Context::Ptr c, GLenum target = GL_TEXTURE_2D, GLuint internalFormat = GL_RGBA8)
                :Target(target),InternalFormat(internalFormat),Object(c,&creator,&deleter)
                {
                }

//                 ~Texture()
//                 {
//                     if(id)
//                     {
//                         glDeleteTextures(1,&id);
//                         id = 0;
//                     }
//                 }

                void Bind()
                {
                    Bind(_target);
                }

                void Bind(GLenum target)
                {
                    SetTarget(target);
/*                    if(!id)
                    {
                        glGenTextures(1,&id);
                        Error();
                        if(!id)
                            throw Exception("Problem generating texture id");
                    }*/
                    glBindTexture(target,id);
                    Error();
                }


                void Unbind()
                {
                    Unbind(_target);
                }

                void Unbind(GLenum target)
                {
                    _target = target;
                    glBindTexture(target,0);
                    Error();
                }


                void Image(GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, GLvoid const * data)
                {
                    SetInternalFormat(internalFormat);
                    
                    switch (_target)
                    {
                        case GL_TEXTURE_1D:
                        case GL_PROXY_TEXTURE_1D:
                            glTexImage1D(_target, level, internalFormat, width, border, format, type, data);
                            break;
                        case GL_TEXTURE_2D:
                        case GL_PROXY_TEXTURE_2D:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                        case GL_PROXY_TEXTURE_CUBE_MAP:
                            glTexImage2D(_target, level, internalFormat, width, height, border, format, type, data);
                            break;
                        case GL_TEXTURE_3D:
                        case GL_PROXY_TEXTURE_3D:
                            glTexImage3D(_target, level, internalFormat, width, height, depth, border, format, type, data);
                            break;
                        default:
                            throw Exception("Unsupported texture target");
                    }
                    Error();
                }
                

                void Image(GLsizei width, GLsizei height = 0, GLsizei depth = 0, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE)
                {
                    Image(0,internalFormat,width,height,depth,0,format,type,NULL);
                }
                

                /// Creates an OpenGL texture from the raster.
                template <typename T> void Image(T const &raster, GLint level = 0);
                //void Image(Raster<Pixel<RGB8> > const &raster, GLint level = 0);
                //void Image(Raster<Pixel<RGBA8> > const &raster, GLint level = 0);

                void SubImage(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data)
                {
                    switch (_target)
                    {
                        case GL_TEXTURE_1D:
                        case GL_PROXY_TEXTURE_1D:
                            glTexSubImage1D(_target, level, xoffset, width, format, type, data);
                            break;
                        case GL_TEXTURE_2D:
                        case GL_PROXY_TEXTURE_2D:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                        case GL_PROXY_TEXTURE_CUBE_MAP:
                            glTexSubImage2D(_target, level, xoffset, yoffset, width, height, format, type, data);
                            break;
                        case GL_TEXTURE_3D:
                        case GL_PROXY_TEXTURE_3D:
                            glTexSubImage3D(_target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
                            break;
                        default:
                            throw Exception("Unsupported texture target");
                    }
                    Error();
                }
                

                void SubImage(Raster<Pixel<RGB8> > const &raster, GLint level = 0, GLint xoffset = 0, GLint yoffset = 0)
                {
                    PushClientAttrib a(GL_CLIENT_PIXEL_STORE_BIT);
                    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                    
                    SubImage(level,xoffset,yoffset,0,raster.GetColumns(),raster.GetRows(),0, GL_RGB, GL_UNSIGNED_BYTE, raster.raw());
                }
                
                void SubImage(Raster<Pixel<RGBA8> > const &raster, GLint level = 0, GLint xoffset = 0, GLint yoffset = 0)
                {
                    PushClientAttrib a(GL_CLIENT_PIXEL_STORE_BIT);
                    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
                    
                    SubImage(level,xoffset,yoffset,0,raster.GetColumns(),raster.GetRows(),0, GL_RGBA, GL_UNSIGNED_BYTE, raster.raw());
                }
                

//                 void CopyImage(GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
// 
//                 void CopySubImage(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
// 
//                 void GetImage(GLint level, GLenum format, GLenum type, GLvoid * img);
//                 void GetImage(Raster<Pixel<3> > &raster, GLint level = 0);
//                 void GetImage(Raster<Pixel<4> > &raster, GLint level = 0);

                void Parameter(GLenum pname, GLint param)
                {
                    glTexParameteri(_target, pname, param);
                    Error();
                }

                void Parameter(GLenum pname, GLfloat param)
                {
                    glTexParameterf(_target, pname, param);
                    Error();
                }

                void Parameter(GLenum pname, std::vector<GLint> const &param)
                {
                    glTexParameteriv(_target, pname, &param.front());
                    Error();
                }

                void Parameter(GLenum pname, std::vector<GLfloat> const &param)
                {
                    glTexParameterfv(_target, pname, &param.front());
                    Error();
                }

                void WrapS(GLint param){Parameter(GL_TEXTURE_WRAP_S,param);}
                void WrapT(GLint param){Parameter(GL_TEXTURE_WRAP_T,param);}
                void MinFilter(GLint param){Parameter(GL_TEXTURE_MIN_FILTER,param);}
                void MagFilter(GLint param){Parameter(GL_TEXTURE_MAG_FILTER,param);}
        };

        template <typename T > inline void Texture::Image(T const &raster, GLint level)
        {
            PushClientAttrib a(GL_CLIENT_PIXEL_STORE_BIT);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);

            std::pair<GLenum,GLenum> format = PixelFormat<typename T::value_type::pixel_format>();

            Image(level, internalFormat, raster.GetColumns(), raster.GetRows(), 0, 0, format.first, format.second, raster.raw());
        }
    }
}

#endif

