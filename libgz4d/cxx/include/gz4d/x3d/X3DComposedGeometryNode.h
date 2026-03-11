// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DCOMPOSEDGEOMETRYNODE_H
#define GZ4D_X3DCOMPOSEDGEOMETRYNODE_H

#include "X3DGeometryNode.h"

namespace gz4d
{

    class X3DComposedGeometryNode: public X3DGeometryNode
    {
    public:
        X3DComposedGeometryNode();
        void Load(const boost::property_tree::ptree &x3d, X3D* root);
        void Draw();
    protected:
        void ApplyNormal(SFInt32 index);
        void ApplyColor(SFInt32 index);
        void ApplyTexCoord(SFInt32 index);
        virtual void Draw(SFInt32 index);
        void DrawNormal(SFInt32 index, SFInt32 ni);
        virtual void GenerateNormals()=0;

        SFNode color;
        SFNode coord;
        SFNode normal;
        SFNode texCoord;
        SFBool ccw;
        SFBool colorPerVertex;
        SFBool normalPerVertex;
        SFBool solid;

        std::vector<gz4d::Vector<float,3> > generated_normals;
    };
};

#endif
