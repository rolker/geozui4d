// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_TARGET_H
#define GZ4D_GLO_TARGET_H

#include <gz4d/glo.h>

namespace gz4d
{
    namespace glo
    {
        class Framebuffer;

        /// \ingroup glo
        class Target
        {
            protected:
                friend class Framebuffer;
                GLenum _target;
            public:
                Target(GLenum target)
                :_target(target)
                {
                }

                void SetTarget(GLenum target)
                {
                    _target = target;
                }
        };
    }
}

#endif
