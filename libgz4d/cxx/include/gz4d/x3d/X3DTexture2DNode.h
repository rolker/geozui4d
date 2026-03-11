// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DTEXTURE2DNODE_H
#define GZ4D_X3DTEXTURE2DNODE_H

#include "X3DTextureNode.h"

namespace gz4d
{
    class X3DTexture2DNode : public X3DTextureNode
    {
        public:
            X3DTexture2DNode();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
        protected:
            SFBool repeatS;
            SFBool repeatT;
    };
};
#endif
