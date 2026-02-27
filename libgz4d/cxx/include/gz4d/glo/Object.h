#ifndef GZ4D_GLO_OBJECT_H
#define GZ4D_GLO_OBJECT_H

#include <gz4d/glo.h>
#include <cassert>
#include "Error.h"
#include "Context.h"

namespace gz4d
{
    namespace glo
    {
        class Framebuffer;
        class Program;

        /// \ingroup glo
        class Object
        {
            protected:
                friend class Framebuffer;
                friend class Program;
                
                //GLuint id;

                Context::ID::Token id;
            public:
                Object(Context::Ptr c, GLuint(*idCreator)(), void (*idDeleter)(GLuint))
                :id(c->createID(idCreator,idDeleter))
                {
                }

                operator bool()
                {
                    return id.isAllocated();
                }
        };
    }
}

#endif
