// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef EK500_H
#define EK500_H

#include "engine/Object.h"

class EK500: public Object
{
public:
    EK500();
    DERIVE_OBJECT(EK500);
    int perform(int argc, const char **argv, Tcl_Interp *interp);
    void DrawLowRes(float degradation, Window3D *win);
    void getHtmlInfo(std::stringstream &html, short level);
private:
    struct Target
    {
        float time;
        gz4d::Point<float> position;
        float value;
    };

    class Attributes
    {
        public:
            Attributes();
            float time;
            Point4f color;
            float size;
    };

    class TimeZone
    {
        public:
            TimeZone();
            Attributes start;
            Attributes end;
    };

    std::vector<Target> targets;
    Attributes attributes;
    std::vector<TimeZone> zones;

    double baseTime;
    double basePosition[2];
};


#endif
