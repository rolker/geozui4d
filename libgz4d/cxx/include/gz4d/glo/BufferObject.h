// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_BUFFEROBJECT_H
#define GZ4D_GLO_BUFFEROBJECT_H

#include <gz4d/glo/Extension.h>
#include <gz4d/glo/Target.h>
#include <gz4d/glo/Object.h>
#include <gz4d/glo/Error.h>


namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class Buffer: public Target, public Object, protected Extension
        {
            static GLuint creator()
            {
                GLuint ret;
                glGenBuffers(1,&ret);
                return ret;
            }
            
            static void deleter(GLuint i)
            {
                glDeleteBuffers(1,&i);
            }
            
            public:
                Buffer(Context::Ptr c, GLenum target)
                :Target(target),Object(c,&creator,&deleter)
                {
                }

               void Bind()
                {
/*                    if(!id)
                    {
                        switch(_target)
                        {
                            case GL_PIXEL_PACK_BUFFER:
                            case GL_PIXEL_UNPACK_BUFFER:
                                Init("GL_VERSION_2_1");
                                break;
                            default:
                                Init("GL_VERSION_1_5");
                        }
                        glGenBuffers(1,&id);
                        Error();
                        if(!id)
                            throw Exception("Problem generating Buffer id");
                    }*/
                    glBindBuffer(_target,id);
                    Error();
                }

                void Bind(GLenum target)
                {
                    SetTarget(target);
                    Bind();
                }

                void Unbind()
                {
                    glBindBuffer(_target,0);
                    Error();
                }

                void Unbind(GLenum target)
                {
                    SetTarget(target);
                    Unbind();
                }


                void Data(GLsizeiptr size, GLvoid const* data, GLenum usage = GL_STATIC_DRAW)
                {
                    glBufferData(_target,size,data,usage);
                    Error();
                }

                void SubData(GLintptr offset, GLsizeiptr size, GLvoid const* data)
                {
                    glBufferSubData(_target, offset, size, data);
                    Error();
                }
                

                void * Map(GLenum access = GL_READ_WRITE)
                {
                    void *ret = glMapBuffer(_target, access);
                    Error();
                    return ret;
                }

                void Unmap()
                {
                    glUnmapBuffer(_target);
                    Error();
                }

                static Buffer CreateArrayBuffer(Context::Ptr c)
                {
                    return Buffer(c,GL_ARRAY_BUFFER);
                }

                static Buffer CreateElementArrayBuffer(Context::Ptr c)
                {
                    return Buffer(c,GL_ELEMENT_ARRAY_BUFFER);
                }

                static Buffer CreatePixelPackBuffer(Context::Ptr c)
                {
                    return Buffer(c,GL_PIXEL_PACK_BUFFER);
                }

                static Buffer CreatePixelUnpackBuffer(Context::Ptr c)
                {
                    return Buffer(c, GL_PIXEL_UNPACK_BUFFER);
                }
        };

    }
}

#endif
