// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DINDEXEDFACESET
#define GZ4D_X3DINDEXEDFACESET

#include "X3DComposedGeometryNode.h"

namespace gz4d
{
    class X3DIndexedFaceSet : public X3DComposedGeometryNode
    {
    public:
        X3DIndexedFaceSet();
        void Load(const boost::property_tree::ptree &x3d, X3D* root);
        virtual void Draw();
    protected:
        void GenerateNormals();

        MFInt32 colorIndex;
        MFInt32 coordIndex;
        SFFloat creaseAngle;
        MFInt32 normalIndex;
        MFInt32 texCoordIndex;
        X3D* r;
    };
};

#endif
