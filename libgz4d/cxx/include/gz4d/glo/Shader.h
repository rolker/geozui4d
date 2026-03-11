// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_SHADER_H
#define GZ4D_GLO_SHADER_H

#include <gz4d/glo/Object.h>
#include <gz4d/glo/Extension.h>

namespace gz4d
{
    namespace glo
    {
        template <int ST> class Shader: public Object, protected Extension
        {
            static GLuint creator()
            {
                return glCreateShader(ST);
            }

            static void deleter(GLuint i)
            {
                glDeleteShader(i);
            }
            
        public:
            Shader(Context::Ptr c):Object(c, &creator, &deleter){}

            void Source(std::string const &src)
            {
                char const * s = src.c_str();
                glShaderSource(id,1,&s,NULL);
            }

            void Compile() const
            {
                glCompileShader(id);
            }

            operator bool() const
            {
                return glIsShader(id);
            }

            bool CompileStatus() const
            {
                int ret;
                glGetShaderiv(id,GL_COMPILE_STATUS,&ret);
                return ret;
            }

            std::string GetInfoLog() const
            {
                int logLength;
                glGetShaderiv(id,GL_INFO_LOG_LENGTH,&logLength);
                if(logLength)
                {
                    char * log = new char[logLength];
                    glGetShaderInfoLog(id,logLength,NULL,log);
                    std::string ret(log);
                    delete[] log;
                    return ret;
                }
                return std::string();
            }

            std::string GetSource() const
            {
                int srcLength;
                glGetShaderiv(id,GL_SHADER_SOURCE_LENGTH,&srcLength);
                if(srcLength)
                {
                    char * src = new char[srcLength];
                    glGetShaderSource(id,srcLength,NULL,src);
                    std::string ret(src);
                    delete[] src;
                    return ret;
                }
                return std::string();
            }

        };
    }
}

#endif
