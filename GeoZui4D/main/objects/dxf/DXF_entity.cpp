/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"DXF_entity.h"
#include<string.h>
#include<math.h>
#include"DXF_VERTEX.h"
#include "engine/ObjectPoseState.h"
#include<stdlib.h>
#include "utilities/Utilities.h"
#include <gz4d/vector/operations.h>

DXF_entity::DXF_entity()
    :color(1.0,1.0,1.0),hasColor(false),OCSnormal(0.0,0.0,1.0),firstFace(0)
{
}

int DXF_entity::processEntry()
{
    int i;

    if(DXF_thing::processEntry())
        return 1;
	
    switch(groupCode)
    {
        case 62:
            i = atoi(valueString)-1;
            color = gz4d::Color(red[i],green[i],blue[i]);
            hasColor = true;
            return 1;
        case 210:
            OCSnormal[0]=(float)atof(valueString);
            return 1;
        case 220:
            OCSnormal[1]=(float)atof(valueString);
            return 1;
        case 230:
            OCSnormal[2]=(float)atof(valueString);
            return 1;
        default:
            return 0;
    }
    return 0;
}

#define ONE64TH 0.015625

void DXF_entity::gotoOCS()
{
    gz4d::Vector<float,3> x,y;
	
    if(OCSnormal[2]<1.0)
    {
        if(OCSmatrix.empty())
        {
            OCSmatrix.resize(16);
            if(fabs(OCSnormal[0])< ONE64TH&&fabs(OCSnormal[1])<ONE64TH)
            {
                y[0]=0.0;y[1]=1.0;y[2]=0.0; // y is filling in as a y axis for now
                x = normalize(cross(y,OCSnormal));
            }
            else
            {
                y[0]=0.0;y[1]=0.0;y[2]=1.0; // y is filling in as a z axis for now
                x = normalize(cross(y,OCSnormal));
            }
            y = normalize(cross(OCSnormal,x));
            OCSmatrix[0]=x[0];
            OCSmatrix[1]=x[1];
            OCSmatrix[2]=x[2];
            OCSmatrix[3]=0.0;
            OCSmatrix[4]=y[0];
            OCSmatrix[5]=y[1];
            OCSmatrix[6]=y[2];
            OCSmatrix[7]=0.0;
            OCSmatrix[8]=OCSnormal[0];
            OCSmatrix[9]=OCSnormal[1];
            OCSmatrix[10]=OCSnormal[2];
            OCSmatrix[11]=0.0;
            OCSmatrix[12]=0.0;
            OCSmatrix[13]=0.0;
            OCSmatrix[14]=0.0;
            OCSmatrix[15]=1.0;
        }
        glMultMatrixf(&OCSmatrix.front());
    }
}

void DXF_entity::processVerticies()
{
    while(groupCode==0&&strcmp(valueString,"VERTEX")==0)
    {
        DXF_VERTEX v;
        v.readFrom();
        vertex.push_back(v);
        if(groupCode==0&&strcmp(valueString,"SEQEND")==0)
            readNext();
    }
}

int DXF_entity::getFirstFace()
{
    return firstFace;
}

