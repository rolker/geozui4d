// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include"DXF_LINE.h"
#include<stdlib.h>
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/Color.h>

int DXF_LINE::readFrom(FILE * file)
{
    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 10:
                    start[0]=(float)atof(valueString);
                    break;
                case 20:
                    start[1]=(float)atof(valueString);
                    break;
                case 30:
                    start[2]=(float)atof(valueString);
                    break;
                case 11:
                    end[0]=(float)atof(valueString);
                    break;
                case 21:
                    end[1]=(float)atof(valueString);
                    break;
                case 31:
                    end[2]=(float)atof(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
    }

    bbox.set(gz4d::Point<double>(start));
    bbox.expand(gz4d::Point<double>(start));

    return 0;
}

void DXF_LINE::Draw(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_LIGHTING_BIT);
    if(hasColor)
        gz4d::glo::Color(color);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
        gz4d::glo::Vertex(start);
        gz4d::glo::Vertex(end);
    glEnd();
}

