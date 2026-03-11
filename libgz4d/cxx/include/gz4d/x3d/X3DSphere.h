// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DSPHERE_H
#define GZ4D_X3DSPHERE_H

#include "X3DGeometryNode.h"

namespace gz4d
{
    class X3DSphere : public X3DGeometryNode
    {
        public:
            X3DSphere();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
        protected:
            SFFloat radius;
            SFBool solid;
    };
};

#endif
