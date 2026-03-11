// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
