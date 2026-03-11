// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DNormal.h>
#include <gz4d/glo/Vertex.h>

gz4d::X3DNormal::X3DNormal()
{
    typeName = "Normal";
}

void gz4d::X3DNormal::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadArray( x3d, "vector", vector);
    X3DNormalNode::Load( x3d, root);
}

void gz4d::X3DNormal::Draw(SFInt32 index)
{
    if (index < (SFInt32)(vector.size()))
    {
        glNormal3fv(&(vector[index][0]));
    }
}

void gz4d::X3DNormal::Draw(SFInt32 index, SFVec3f o)
{
    if (index < (SFInt32)(vector.size()))
    {
        o += vector[index];
        glo::Vertex(o);
    }
}
