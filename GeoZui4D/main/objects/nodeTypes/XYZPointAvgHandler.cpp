/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "objects/nodeTypes/XYZPointAvgHandler.h"
#include <stdlib.h>
#include <typeinfo>
#include <gz4d/util/Utilities.h>

XYZPointAvgHandler::XYZPointAvgHandler()
{
    total = 0.0;
    altTotal = 0.0;
    count = 0;
}

void XYZPointAvgHandler::Add(SDV_Data &s)
{
    count++;
    total += s.sounding[2];
    value = total/(double)count;
    if (!gz4d::IsNan(s.alternateData))
    {
        altTotal += s.alternateData;
        alternateData = altTotal/count;
    }
}

void XYZPointAvgHandler::Draw()
{
}

void XYZPointAvgHandler::Draw(double minx, double maxx, double miny, double maxy)
{
}

void XYZPointAvgHandler::writeTo(FILE * outfile)
{
}
