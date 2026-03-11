// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DCone.h>

#include <cmath>
//#include <gz4d/util/Xml.h>

gz4d::X3DCone::X3DCone()
{
    bottom = true;
    height = 2.0;
    bottomRadius = 1.0;
    side = true;
    solid = true;
    typeName = "Cone";
}

void gz4d::X3DCone::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"solid",solid);
    LoadItem(x3d,"bottom",bottom);
    LoadItem(x3d,"side",side);
    LoadItem(x3d,"height",height);
    LoadItem(x3d,"bottomRadius",bottomRadius);
//     x3d.getBool( solid, "solid");
//     x3d.getBool( bottom, "bottom");
//     x3d.getBool( side, "side");
// 
//     x3d.getFloat( height, "height");
//     x3d.getFloat( bottomRadius, "bottomRadius");

    X3DGeometryNode::Load( x3d, root);
}

void gz4d::X3DCone::Draw()
{
    if (preDraw())
        return;

    //bottom
    if (bottom)
    {
        glBegin(GL_POLYGON);
        glNormal3f(0.0,-1.0,0.0);
        float t = -height/2.0;
        for(float a = 2.0; a >= 0; a-=0.1)
        {
            float r((a-0.5)*M_PI);
            float x(cosf(r));
            float y (sinf(r));
            glTexCoord2f(0.5+(x/2.0),0.5+(-y/2.0));
            glVertex3f(x*bottomRadius,t,-y*bottomRadius);
        }
        glEnd();
    }

    //side
    if (side)
    {
        glBegin(GL_QUAD_STRIP);
        float t = height/2.0;
        for(float a = 0.0; a <= 2.005; a+=0.1)
        {
            float r = (a+0.5)*M_PI;
            float x = cosf(r);
            float z = sinf(r);
            glNormal3f(x,bottomRadius/height,-z);
            glTexCoord2f(a/2.0,1.0);
            glVertex3f(0.0,t,0.0);
            glTexCoord2f(a/2.0,0.0);
            glVertex3f(x*bottomRadius,-t,-z*bottomRadius);
        }
        glEnd();
    }
}
