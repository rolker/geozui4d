// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_PUSHCLIENTATTRIB_H
#define GZ4D_GLO_PUSHCLIENTATTRIB_H

#include <gz4d/glo.h>
namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class PushClientAttrib
        {
            public:
                PushClientAttrib(GLbitfield mask=GL_CLIENT_ALL_ATTRIB_BITS){glPushClientAttrib(mask);}
                ~PushClientAttrib(){glPopClientAttrib();}
        };
    }
}

#endif
