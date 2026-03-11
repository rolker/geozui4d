// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DBox.h>

//#include <gz4d/util/Xml.h>

gz4d::X3DBox::X3DBox()
    :size(2.0),solid(true)
{
    typeName = "Box";
}

void gz4d::X3DBox::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"solid",solid);
    LoadItem(x3d,"size",size);
    X3DGeometryNode::Load( x3d, root);
    pos = size;
    pos /= 2.0;
}

void gz4d::X3DBox::Draw()
{
    if (preDraw())
        return;

    glBegin(GL_QUADS);
        glNormal3f(0.0,0.0,1.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(-pos[0],pos[1],pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(-pos[0],-pos[1],pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(pos[0],-pos[1],pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(pos[0],pos[1],pos[2]);

        glNormal3f(1.0,0.0,0.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(pos[0],pos[1],pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(pos[0],-pos[1],pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(pos[0],pos[1],-pos[2]);

        glNormal3f(0.0,0.0,-1.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(pos[0],pos[1],-pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(-pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(-pos[0],pos[1],-pos[2]);

        glNormal3f(-1.0,0.0,0.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(-pos[0],pos[1],-pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(-pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(-pos[0],-pos[1],pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(-pos[0],pos[1],pos[2]);

        glNormal3f(0.0,1.0,0.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(-pos[0],pos[1],-pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(-pos[0],pos[1],pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(pos[0],pos[1],pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(pos[0],pos[1],-pos[2]);

        glNormal3f(0.0,-1.0,0.0);
        glTexCoord2f(0.0,1.0);
        glVertex3f(-pos[0],-pos[1],pos[2]);
        glTexCoord2f(0.0,0.0);
        glVertex3f(-pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,0.0);
        glVertex3f(pos[0],-pos[1],-pos[2]);
        glTexCoord2f(1.0,1.0);
        glVertex3f(pos[0],-pos[1],pos[2]);
    glEnd();
}
