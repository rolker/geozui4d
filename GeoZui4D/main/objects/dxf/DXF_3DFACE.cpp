/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"DXF_3DFACE.h"
#include<stdlib.h>
#include"utilities/Utilities.h"
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/glo/Normal.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/vector/operations.h>

DXF_3DFACE::DXF_3DFACE()
    :points(4),normal(0.0,0.0,1.0),fourthFlag(false)
{
}

// documented in Object
int DXF_3DFACE::readFrom(FILE * file)
{
    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 10:
                    points[0][0]=(float)atof(valueString);
                    break;
                case 20:
                    points[0][1]=(float)atof(valueString);
                    break;
                case 30:
                    points[0][2]=(float)atof(valueString);
                    break;
                case 11:
                    points[1][0]=(float)atof(valueString);
                    break;
                case 21:
                    points[1][1]=(float)atof(valueString);
                    break;
                case 31:
                    points[1][2]=(float)atof(valueString);
                    break;
                case 12:
                    points[2][0]=(float)atof(valueString);
                    break;
                case 22:
                    points[2][1]=(float)atof(valueString);
                    break;
                case 32:
                    points[2][2]=(float)atof(valueString);
                    break;
                case 13:
                    points[3][0]=(float)atof(valueString);
                    fourthFlag = 1;
                    break;
                case 23:
                    points[3][1]=(float)atof(valueString);
                    break;
                case 33:
                    points[3][2]=(float)atof(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
    }

    gz4d::Vector<float,3> a = points[0] - points[1];
    gz4d::Vector<float,3> b = points[1] - points[2];
    normal = normalize(cross(a,b));

    bbox.set(gz4d::Point<double>(points[0]));

    for(int i=1; i<4 || (!fourthFlag && i<3); i++)
    {
        bbox.expand(gz4d::Point<double>(points[i]));
    }

    return 0;
}

void DXF_3DFACE::Draw(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_LIGHTING_BIT);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
    glBegin(GL_QUADS);
    gz4d::glo::Normal(normal);
    gz4d::glo::Vertex(points[0]);
    gz4d::glo::Vertex(points[1]);
    gz4d::glo::Vertex(points[2]);
    if(fourthFlag)
        gz4d::glo::Vertex(points[3]);
    glEnd();
}


