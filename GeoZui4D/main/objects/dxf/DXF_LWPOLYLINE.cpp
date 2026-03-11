// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include"DXF_LWPOLYLINE.h"
#include<stdlib.h>
#include <gz4d/glo/Color.h>

DXF_LWPOLYLINE::DXF_LWPOLYLINE()
    :flags(0),elevation(0.0)
{
}

// documented in Object
int DXF_LWPOLYLINE::readFrom(FILE * file)
{
    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 10:
                    points.push_back(gz4d::Point<float>((float)atof(valueString),0.0,0.0));
                    break;
                case 20:
                    points.back()[1]=(float)atof(valueString);
                    break;
                case 38:
                    elevation = (float)atof(valueString);
                    break;
/*                case 90:
                    pCount=atoi(valueString);
                    points = new Point3f[pCount];
                    break;*/
                case 70:
                    flags=atoi(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
    }
    return 0;
}

void DXF_LWPOLYLINE::Draw(float degradation, Window3D *win)
{
    if(hasColor)
        gz4d::glo::Color(color);
    glPushMatrix();
    gotoOCS();
    glDisable(GL_LIGHTING);
    if(flags&1)
    {
        glBegin(GL_LINE_LOOP);
    }
    else
    {
        glBegin(GL_LINE_STRIP);
    }
    for(std::vector<gz4d::Point<float> >::iterator i = points.begin(); i != points.end(); ++i)
        glVertex3f((*i)[0],(*i)[1],elevation);
    glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}
