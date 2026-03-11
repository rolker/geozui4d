// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_FRAMEBUFFEROBJECT_H
#define GZ4D_FRAMEBUFFEROBJECT_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>
#include <gz4d/glo/Extension.h>
#include <gz4d/glo/InternalFormat.h>
#include <gz4d/glo/Object.h>

#include <gz4d/glo/Texture.h>

namespace gz4d
{

    namespace glo
    {
        /// \ingroup glo
        class Renderbuffer: public Target, public InternalFormat, public Object, protected Extension
        {
            static GLuint creator()
            {
                GLuint ret;
                glGenRenderbuffersEXT(1,&ret);
                return ret;
            }
            
            static void deleter(GLuint i)
            {
                glDeleteRenderbuffersEXT(1,&i);
            }
            
            public:
                typedef std::shared_ptr<Renderbuffer> Ptr;

                Renderbuffer(Context::Ptr c, GLuint internalFormat = GL_RGBA8)
                :Target(GL_RENDERBUFFER),InternalFormat(internalFormat),Object(c,&creator,&deleter)
                {
                }
                
               void Bind()
                {
/*                    Init("GL_EXT_framebuffer_object");
                    if(!id)
                    {
                        glGenRenderbuffersEXT(1,&id);
                        Error();
                        if(!id)
                            throw Exception("Problem generating Renderbuffer id");
                    }*/
                    glBindRenderbufferEXT(_target,id);
                    Error();
                }

                void Bind(GLenum target)
                {
                    SetTarget(target);
                    Bind();
                }

                void Unbind()
                {
                    glBindRenderbufferEXT(_target,0);
                    Error();
                }

                void Unbind(GLenum target)
                {
                    SetTarget(target);
                    Unbind();
                }


                void Storage(GLsizei width, GLsizei height)
                {
                    glRenderbufferStorageEXT(_target,internalFormat,width,height);
                    Error();
                }

                void Storage(GLsizei samples, GLsizei width, GLsizei height)
                {
                    Init("GL_EXT_framebuffer_multisample");
                    Init("GL_EXT_framebuffer_blit");
                    #ifdef GZ4D_GLEW
                    glRenderbufferStorageMultisampleEXT(_target,samples,internalFormat,width,height);
                    #else
                    glRenderbufferStorageMultisample(_target,samples,internalFormat,width,height);
                    #endif
                    Error();
                }

                static GLint GetMaxSize()
                {
                    Init("GL_EXT_framebuffer_object");
                    GLint ret;
                    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&ret);
                    return ret;
                }

                static Renderbuffer CreateDepthBuffer(Context::Ptr c, GLuint internalFormat = GL_DEPTH_COMPONENT32)
                {
                    return Renderbuffer(c, internalFormat);
                }

                static Renderbuffer CreateStencilBuffer(Context::Ptr c, GLuint internalFormat = GL_STENCIL_INDEX16)
                {
                    return Renderbuffer(c, internalFormat);
                }
        };

        /** Manages an off screen rendering buffer.
        If the Framebuffer Object extension is available, this class
        will create a framebuffer object in which color buffers and
        an optional depth and/or stencil buffer can be created.
        See https://vislab-ccom.unh.edu/dev/GeoZui4D/wiki/DevArea/FrameBufferObjects for more info.
        \ingroup glo
        **/
        class Framebuffer: public Target, protected Extension, protected Object
        {
            static GLuint creator()
            {
                GLuint ret;
                glGenFramebuffersEXT(1,&ret);
                return ret;
            }
            
            static void deleter(GLuint i)
            {
                glDeleteFramebuffersEXT(1,&i);
            }
            
            public:
                typedef std::shared_ptr<Framebuffer> Ptr;

                Framebuffer(Context::Ptr c, GLenum target = GL_FRAMEBUFFER)
                :Target(target),Object(c, &creator, &deleter)
                {
                }

                ///Switches to this framebuffer object for rendering and/or reading pixels.
                void Bind()
                {
/*                    if(!id)
                    {
                        Init("GL_EXT_framebuffer_object");
                        glGenFramebuffersEXT(1,&id);
                        Error();
                        if(!id)
                            throw Exception("Unable to reserve FramebufferObject ID");
                    }*/
                    glBindFramebufferEXT(_target,id);
                    Error();
                }
                
                void Bind(GLenum target)
                {
                    SetTarget(target);
                    Bind();
                }
                

                ///Switches back to the main framebuffer.
                void Unbind()
                {
                    glBindFramebufferEXT(_target,0);
                    Error();
                }
                
                void Unbind(GLenum target)
                {
                    SetTarget(target);
                    Unbind();
                }
                

                ///Query our "completeness", or our readiness for use.
                ///A framebuffer can be "incomplete" if it contains renderbuffers or texture
                ///images of different size or pizel formats.
                void CheckStatus()
                {
                    GLenum status = glCheckFramebufferStatusEXT(_target);
                    switch (status)
                    {
                        case GL_FRAMEBUFFER_COMPLETE:
                            return;
                        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                            throw Exception("Framebuffer: Incomplete Attachement");
                        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                            throw Exception("Framebuffer: Incomplete Missing Attachement");
                        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                            throw Exception("Framebuffer: Incomplete Draw Buffer");
                        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                            throw Exception("Framebuffer: Incomplete Read Buffer");
                        case GL_FRAMEBUFFER_UNSUPPORTED:
                            throw Exception("Framebuffer: Unsupported");
                        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                            throw Exception("Framebuffer: Incomplete Multisample");
                        default:
                            throw Exception("Framebuffer: Undefined");
                    }
                }
                

                /// Query our bound state.
                /// @return True if we are bound.
                bool IsBound()
                {
                    GLint current;
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&current);
                    return current == id;
                }
                

                void Attach(GLenum attachment, Texture const &texture, GLint level=0, GLint layer=0)
                {
                    switch (texture._target)
                    {
                        case GL_TEXTURE_1D:
                            glFramebufferTexture1DEXT(_target, attachment, texture._target, texture.id, level);
                            break;
                        case GL_TEXTURE_2D:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
                        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
                        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                            glFramebufferTexture2DEXT(_target, attachment, texture._target, texture.id, level);
                            break;
                        case GL_TEXTURE_3D:
                            glFramebufferTexture3DEXT(_target, attachment, texture._target, texture.id, level, layer);
                            break;
                        default:
                            throw Exception("Unsupported texture target");
                    }
                    Error();
                    
                }
                
                void Attach(GLenum attachment, Renderbuffer const &buffer)
                {
                    glFramebufferRenderbufferEXT(_target, attachment, buffer._target, buffer.id);
                    Error();
                }
                
                

                void Attach(Texture const &texture, GLint level=0, GLint layer=0)
                {
                    Attach(GL_COLOR_ATTACHMENT0,texture,level,layer);
                }

                void Attach(Renderbuffer const &buffer)
                {
                    switch (buffer.internalFormat)
                    {
                        case GL_DEPTH_COMPONENT:
                        case GL_DEPTH_COMPONENT16:
                        case GL_DEPTH_COMPONENT24:
                        case GL_DEPTH_COMPONENT32:
                            Attach(GL_DEPTH_ATTACHMENT,buffer);
                            break;
                        case GL_STENCIL_INDEX1:
                        case GL_STENCIL_INDEX4:
                        case GL_STENCIL_INDEX8:
                        case GL_STENCIL_INDEX16:
                            Attach(GL_STENCIL_ATTACHMENT,buffer);
                            break;
                        case GL_DEPTH_STENCIL:
                        case GL_DEPTH24_STENCIL8:
                            Attach(GL_DEPTH_ATTACHMENT,buffer);
                            Attach(GL_STENCIL_ATTACHMENT,buffer);
                            break;
                        default:
                            Attach(GL_COLOR_ATTACHMENT0,buffer);
                    }
                }
        };
    }
}

#endif
