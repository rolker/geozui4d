// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
