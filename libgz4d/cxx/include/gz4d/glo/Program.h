#ifndef GZ4D_GLO_PROGRAM_H
#define GZ4D_GLO_PROGRAM_H

#include <gz4d/glo/Object.h>
#include <gz4d/glo/Extension.h>
#include "Shader.h"
#include "Uniform.h"
#include "VertexAttribute.h"

namespace gz4d
{
    namespace glo
    {
        class Program: public Object, protected Extension
        {
            static GLuint creator()
            {
                return glCreateProgram();
            }
            
            static void deleter(GLuint i)
            {
                glDeleteProgram(i);
            }
            
            
        public:            
            Program(Context::Ptr c):Object(c, &creator, &deleter){}

            template <int ST> void AttachShader(Shader<ST> const &s)
            {
                glAttachShader(id,s.id);
            }

            template <int ST> void DetachShader(Shader<ST> const &s)
            {
                glDetachShader(id,s.id);
            }

            void Link()
            {
                glLinkProgram(id);
            }

            void Use()
            {
                glUseProgram(id);
            }

            static void FixedFunctionality()
            {
                glUseProgram(0);
            }

            std::string GetInfoLog() const
            {
                int logLength;
                glGetProgramiv(id,GL_INFO_LOG_LENGTH,&logLength);
                if(logLength)
                {
                    char * log = new char[logLength];
                    glGetProgramInfoLog(id,logLength,NULL,log);
                    std::string ret(log);
                    delete[] log;
                    return ret;
                }
                return std::string();
            }

            bool LinkStatus() const
            {
                int ret;
                glGetProgramiv(id,GL_LINK_STATUS,&ret);
                return ret;
            }

            bool Validate() const
            {
                glValidateProgram(id);
                int ret;
                glGetProgramiv(id,GL_VALIDATE_STATUS,&ret);
                return ret;
            }

            int ActiveAttributesCount() const
            {
                int ret;
                glGetProgramiv(id,GL_ACTIVE_ATTRIBUTES,&ret);
                return ret;
            }

            VertexAttribute GetAttribute(std::string const &a) const
            {
                return VertexAttribute(glGetAttribLocation(id, a.c_str()));
            }

            int ActiveUniformsCount() const
            {
                int ret;
                glGetProgramiv(id,GL_ACTIVE_UNIFORMS,&ret);
                return ret;
            }

            Uniform::Info GetActiveUniform(int i) const
            {
                Uniform::Info ret;
                int uniformLength;
                glGetProgramiv(id,GL_ACTIVE_UNIFORM_MAX_LENGTH,&uniformLength);
                if(uniformLength)
                {
                    char * uniform = new char[uniformLength];
                    glGetActiveUniform(id,i,uniformLength,NULL,&ret.size,&ret.type,uniform);
                    ret.name = uniform;
                    delete[] uniform;
                }
                return ret;                
            }

            Uniform GetUniform(std::string const &u) const
            {
                return Uniform(glGetUniformLocation(id,u.c_str()));
            }

        };
    }
}

#endif