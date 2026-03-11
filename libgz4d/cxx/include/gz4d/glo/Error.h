// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLERROR_H
#define GZ4D_GLERROR_H

#include <gz4d/gz4d.h>
#include <gz4d/Exception.h>
#include <gz4d/glo.h>
#include <GL/glu.h>
#include <boost/lexical_cast.hpp>

namespace gz4d
{
    namespace glo
    {

        class GZ4DEXPORT Exception: public gz4d::Exception
        {
            public:
                Exception(std::string message):gz4d::Exception(message){}
        };

        /// Checks for OpenGL errors in debug mode.
        /// If an error is detected an exception is thrown.
        /// Doesn't do anything in release mode.
        /// \ingroup glo
        inline void Error()
        {
    #ifdef GZ4D_GLDEBUG
            GLenum err = glGetError();
            if(err != GL_NO_ERROR)
            {
                std::string errstr = boost::lexical_cast<std::string>(err)+": " + reinterpret_cast<const char *>(gluErrorString(err));
                err = glGetError();
                while (err != GL_NO_ERROR)
                {
                    errstr += ", ";
                    errstr += reinterpret_cast<const char *>(gluErrorString(err));
                    err = glGetError();
                }
                throw(Exception(errstr));
            }
    #endif
        }
    }
}

#endif
