// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include "objects/nodeTypes/XYZPointWAHandler.h"
#include <stdlib.h>
#include <typeinfo>
#include <iostream>
#include <math.h>
#include "objects/SDV_Quadtree.h"
#include <gz4d/util/Utilities.h>

using namespace std;

XYZPointWAHandler::XYZPointWAHandler(double X, double Y)
{
    sum = 0.0;
    weight = 0.0;
    x = X;
    y = Y;
//    isnull = 1;
}

void XYZPointWAHandler::Add(SDV_Data &s)
{
//  LogState ls(&gzlog,"objects/nodeTypes/XYZPointWAHandler::Add",TRACE);

/*    XYZPointWA * swa;
    double distance;
    double w;

    swa = (XYZPointWA *)s;*/

    float distance = sqrt(pow(s.sounding[0]-x,2.0)+pow(s.sounding[1]-y,2.0));

    float r = ((XYZPointWA*)(&s))->radius;
    if(distance < r)
    {
        float w = 1.0-distance;
        weight += w;
        sum += (s.sounding[2]*w);
        value = sum/weight;
        if(!gz4d::IsNan(s.alternateData))
        {
            altSum += s.alternateData*w;
            alternateData = altSum/weight;
//            gzlog << alternateData << endl;
        }
    }
//    gzlog << CERR_POINT(s.sounding) << " sum: " << sum << " w: " << weight << " value: " << value << " d: " << distance << " r: " << r << endl;
}

void XYZPointWAHandler::Draw()
{
    glBegin(GL_POINTS);
        glVertex3d(x,y,value);
    glEnd();
}

void XYZPointWAHandler::writeTo(FILE * outfile)
{
}
