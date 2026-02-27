#ifndef GZ4D_GLO_PUSHATTRIB_H
#define GZ4D_GLO_PUSHATTRIB_H

#include <gz4d/glo.h>
namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class PushAttrib
        {
            public:
                PushAttrib(GLbitfield mask=GL_ALL_ATTRIB_BITS){glPushAttrib(mask);}
                ~PushAttrib(){glPopAttrib();}
        };
    }
}

#endif
