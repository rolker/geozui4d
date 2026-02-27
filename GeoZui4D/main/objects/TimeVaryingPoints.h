/* Copyright (c) 2010 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef TIMEVARYINGPOINTS_H
#define TIMEVARYINGPOINTS_H

#include "engine/Object.h"
#include <gz4d/TimeSequence.h>
#include <gz4d/Point.h>
#include <gz4d/glo/DisplayList.h>

class TimeVaryingPoints : public Object
{
    private:
        struct Value
        {
            double value;
            gz4d::Point<double> offset;
            double age;
            Value();
        };

        struct Point
        {
            gz4d::TimeSequence<Value> values;
        };

        std::vector<Point> points;

        gz4d::Point<double> basePoint;
        double fadeStart;
        double fadeEnd;

        float sizeScale;
        float sizeMin;
        float sizeMax;

        gz4d::glo::DisplayList displayList;

    public:
        TimeVaryingPoints();
        DERIVE_OBJECT(TimeVaryingPoints);
        void DrawLowRes(float degradation, std::shared_ptr< Window3D > win);
        Object::Ptr LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
        void Idle(gz4d::Time time);
};

#endif
