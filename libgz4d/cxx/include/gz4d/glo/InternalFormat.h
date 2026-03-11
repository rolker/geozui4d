// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_INTERNALFORMAT_H
#define GZ4D_GLO_INTERNALFORMAT_H

#include <gz4d/glo.h>

namespace gz4d
{
    namespace glo
    {
        class Framebuffer;

        /// \ingroup glo
        class InternalFormat
        {
            protected:
                friend class Framebuffer;
                GLuint internalFormat;
            public:
                InternalFormat(GLuint format)
                    :internalFormat(format){}
                void SetInternalFormat(GLuint format)
                {
                    internalFormat = format;
                }
        };
    }
}

#endif
