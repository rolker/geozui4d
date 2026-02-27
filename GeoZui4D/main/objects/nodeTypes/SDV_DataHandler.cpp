/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "objects/nodeTypes/SDV_DataHandler.h"

#include "SDV_types.h"
#include "utilities/TimeSequence.h"
#include <gz4d/util/Utilities.h>

SDV_DataHandler::SDV_DataHandler()
{
//    isnull = 1;
    value = gz4d::Nan<float>();
    alternateData = gz4d::Nan<float>();
};

float SDV_DataHandler::GetAverage()
{
    return value;
}

float SDV_DataHandler::GetAlternateData()
{
    return alternateData;
}

float SDV_DataHandler::GetAverage(double t)
{
    if(!gz4d::IsNan(times.getStartTime()))
    {
        if(times.getStartTime()>t)
        {
            return gz4d::Nan<float>();
        }
        else
            return *(float*)(times.get(t));
    }
    else
        return GetAverage();
}

float SDV_DataHandler::GetAlternateData(double t)
{
    if(!gz4d::IsNan(times.getStartTime()))
    {
        if(times.getStartTime()>t)
        {
            return gz4d::Nan<float>();
        }
        else
            return ((float*)(times.get(t)))[1];
    }
    else
        return GetAlternateData();
}

void SDV_DataHandler::Add(SDV_Data &d, double t)
{
    Add(d);

    if(!gz4d::IsNan(t))
    {
        float * a = new float[2];
        a[0] = GetAverage();
        a[1] = GetAlternateData();
//        *a = GetAverage();
        if(!gz4d::IsNan(a[0]))
            times.insert(t,a);
    }
}

double SDV_DataHandler::GetStartValidTime()
{
    return times.getTime();
}

double SDV_DataHandler::GetEndValidTime()
{
    return times.getNextTime();
}

