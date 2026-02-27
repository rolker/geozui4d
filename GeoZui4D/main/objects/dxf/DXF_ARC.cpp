/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"DXF_ARC.h"
#include<math.h>
#include<iostream>
#include <gz4d/glo/Color.h>

using namespace std;

DXF_ARC::DXF_ARC()
{
    center[0]=center[1]=center[2]=0.0;
    startAngle=endAngle=0.0;
    radius=0.0;
}

// documented in Object
int DXF_ARC::readFrom(FILE * file)
{
    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 10:
                    center[0]=(float)atof(valueString);
                    break;
                case 20:
                    center[1]=(float)atof(valueString);
                    break;
                case 30:
                    center[2]=(float)atof(valueString);
                    break;
                case 40:
                    radius=(float)atof(valueString);
                    break;
                case 50:
                    startAngle=(float)atof(valueString);
                    break;
                case 51:
                    endAngle=(float)atof(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
    }
    return 0;
}

void DXF_ARC::Draw(float degradation, Window3D *win)
{
    float x,y;
    float a;
	
    if(endAngle<startAngle)
        endAngle+=360.0;
    if(hasColor)
        gz4d::glo::Color(color);
    if(radius>0.0)
    {
        glPushMatrix();
        gotoOCS();
        glTranslatef(center[0],center[1],center[2]);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINE_STRIP);
        for(a=startAngle;a<endAngle;a+=1.0)
        {
            x = (float)(radius*cos(DEG2RAD(a)));
            y = (float)(radius*sin(DEG2RAD(a)));
            glVertex2f(x,y);
        }
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
}


