// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef UTILITIES_H
#define UTILITIES_H

#include"SDV_types.h"

#include<string>


double interpolateDegrees(double a, double b, double p = .5);
double interpolateValues(double value1, double value2, double weight);
void scaledCross(Point3f a,Point3f b ,Point3f c);

bool isEven(int i);
bool isTrue(std::string s);

bool getGlError();

struct DecimalDegrees
{
    double lat, lon;
    std::string lat_string;
    std::string lon_string;

    DecimalDegrees(double lat, double lon, std::string dblq="\"", bool includeDeg = true);
};

#endif

