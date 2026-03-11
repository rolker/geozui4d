// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DColor.h>

//#include <gz4d/util/Xml.h>

gz4d::X3DColor::X3DColor()
{
    typeName = "Color";
}

void gz4d::X3DColor::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadArray( x3d, "color", color);
    X3DColorNode::Load( x3d, root);
}

void gz4d::X3DColor::Draw()
{
    if (preDraw())
        return;

    glEnable(GL_COLOR_MATERIAL);
    glColorPointer(3,GL_FLOAT,0,&color.front().front());
    glEnableClientState(GL_COLOR_ARRAY);
}

void gz4d::X3DColor::Draw(SFInt32 index)
{
    if (useNode)
        useNode->Draw(index);
    else
    {
        if (color.size() > index)
            glColor3fv(&color[index][0]);
        else
            glColor3fv(&color[0][0]);
    }
}



