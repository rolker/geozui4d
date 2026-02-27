#ifndef GZ4D_GLO_EXTENSION_H
#define GZ4D_GLO_EXTENSION_H

#include <gz4d/glo.h>
#include <gz4d/Exception.h>
#include <string>

namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class Extension
        {
            protected:
                static void Init(std::string const &restriction = std::string())
                {
                    #ifdef GZ4D_GLEW
                    GLenum err = glewInit();
                    if(err != GLEW_OK)
                        throw Exception(reinterpret_cast<char const *>(glewGetErrorString(err)));
                    if (!restriction.empty() && !glewIsSupported(restriction.c_str()))
                        throw Exception(restriction + " not supported.");
                    #endif
                }
        };
    }
}

#endif

