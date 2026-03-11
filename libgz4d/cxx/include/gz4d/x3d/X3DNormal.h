// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DNORMAL_H
#define GZ4D_X3DNORMAL_H

#include "X3DNormalNode.h"

namespace gz4d
{

    class X3DNormal : public X3DNormalNode
    {
        public:
            X3DNormal();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw(SFInt32 index);
            void Draw(SFInt32 index, SFVec3f o);
        protected:
            MFVec3f vector;
    };
};

#endif
