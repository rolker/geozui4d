// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DCylinder.h>

//#include <gz4d/util/Xml.h>
#include <cmath>

gz4d::X3DCylinder::X3DCylinder()
{
    bottom = true;
    height = 2.0;
    radius = 1.0;
    side = true;
    solid = true;
    top = true;
    typeName = "Cylinder";
}

void gz4d::X3DCylinder::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"solid",solid);
    LoadItem(x3d,"bottom",bottom);
    LoadItem(x3d,"side",side);
    LoadItem(x3d,"top",top);
    LoadItem(x3d,"height",height);
    LoadItem(x3d,"radius",radius);

    X3DGeometryNode::Load( x3d, root);
}

void gz4d::X3DCylinder::Draw()
{
    if (preDraw())
        return;

    //top
    if (top)
    {
        glBegin(GL_POLYGON);
            glNormal3f(0.0,1.0,0.0);
            float t = height/2.0;
            for(float a = 0.0; a <= 2.0; a+=0.1)
            {
                float r((a+0.5)*M_PI);
                float x(cosf(r));
                float y (sinf(r));
                glTexCoord2f(0.5+(x/2.0),0.5+(y/2.0));
                glVertex3f(x*radius,t,-y*radius);
            }
        glEnd();
    }
    
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
                glVertex3f(x*radius,t,-y*radius);
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
                float r((a+0.5)*M_PI);
                float x(cosf(r));
                float y (sinf(r));
                glNormal3f(x,0.0,-y);
                glTexCoord2f(a/2.0,1.0);
                glVertex3f(x*radius,t,-y*radius);
                glTexCoord2f(a/2.0,0.0);
                glVertex3f(x*radius,-t,-y*radius);
            }
        glEnd();
    }
}
