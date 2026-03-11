// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DAppearance.h>

#include <gz4d/x3d/X3DMaterial.h>
#include <gz4d/x3d/X3DImageTexture.h>
#include <gz4d/x3d/X3DLineProperties.h>
#include <iostream>
#include <gz4d/glo/Error.h>
#include <gz4d/X3D.h>

gz4d::X3DAppearance::X3DAppearance()
{
    typeName = "Appearance";
}

void gz4d::X3DAppearance::Load(const boost::property_tree::ptree& x3d, gz4d::X3D* root)
{
    for(boost::property_tree::ptree::const_iterator n = x3d.begin(); n != x3d.end(); ++n)
    {
        if (n->first == "Material")
        {
            material = SFNode(new X3DMaterial(),n->second, root);
        }
        if (n->first == "ImageTexture")
        {
            texture = SFNode(new X3DImageTexture(root->context),n->second, root);
            //gzlog << "ImageTexture" << endl;
        }
        if (n->first == "LineProperties")
        {
            lineProperties = SFNode(new X3DLineProperties(),n->second, root);
        }
    }
    X3DAppearanceNode::Load( x3d, root);
}

void gz4d::X3DAppearance::Draw()
{
    if (preDraw())
        return;


    bool lit = true;
    if (material)
        material->Draw();
    else
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(1.0,1.0,1.0);
        lit = false;
    }

    if (texture)
    {
        texture->Draw();
        if(lit)
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        else
            glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
    }

    else
    {
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D);
        glDisable(GL_TEXTURE_CUBE_MAP);
    }

    if (lineProperties)
        lineProperties->Draw();
}
