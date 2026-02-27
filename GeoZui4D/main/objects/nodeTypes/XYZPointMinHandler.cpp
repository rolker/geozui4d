/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "objects/nodeTypes/XYZPointMinHandler.h"
#include <stdlib.h>
#include <typeinfo>
#include <gz4d/util/Utilities.h>

XYZPointMinHandler::XYZPointMinHandler()
{
}

void XYZPointMinHandler::Add(SDV_Data &s)
{
    if (gz4d::IsNan(value) || s.sounding[2]>value)
    {
        value = s.sounding[2];
        if(!gz4d::IsNan(s.alternateData))
            alternateData = s.alternateData;
    }
}


void XYZPointMinHandler::Draw()
{
}

void XYZPointMinHandler::Draw(double minx, double maxx, double miny, double maxy)
{
}

void XYZPointMinHandler::writeTo(FILE * outfile)
{
}
