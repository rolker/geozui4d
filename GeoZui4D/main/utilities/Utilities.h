/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

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

