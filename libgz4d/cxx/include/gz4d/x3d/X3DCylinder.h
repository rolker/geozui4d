// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DCYLINDER_H
#define GZ4D_X3DCYLINDER_H

#include "X3DGeometryNode.h"

namespace gz4d
{
    class X3DCylinder : public X3DGeometryNode
    {
        public:
            X3DCylinder();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
        protected:
            SFBool bottom;
            SFFloat height;
            SFFloat radius;
            SFBool side;
            SFBool solid;
            SFBool top;
    };
};

#endif
