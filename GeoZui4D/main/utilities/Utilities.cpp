// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "utilities/Utilities.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

double interpolateDegrees(double degree1, double degree2, double weight)
{
//	cerr << "interpolateDegrees: " << degree1 << " " << degree2 << endl;

	while(degree1 < degree2-180)

		degree1+=360.0;

	while(degree1 > degree2+180)

		degree1-=360.0;

	return(degree1+((degree2-degree1)*weight));

}

double interpolateValues(double value1, double value2, double weight)
{
	return(value1+((value2-value1)*weight));
}



void scaledCross(Point3f a,Point3f b ,Point3f c)

{

	float factor;

	

	c[0] = a[1]*b[2]-a[2]*b[1];

	c[1] = a[2]*b[0]-a[0]*b[2];

	c[2] = a[0]*b[1]-a[1]*b[0];

	

	factor =sqrtf(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);

	

	c[0]/=factor;

	c[1]/=factor;

	c[2]/=factor;

}



bool isEven(int i)

{

	return !(i%2);

}

bool isTrue(string s)
{
	string l = s;

	for(string::size_type i=0; i<s.size();i++)
		l[i] = tolower(s[i]);

	if(l == "true" || l == "yes" || l == "on" || l == "1" || l == "t" || l == "y")
		return true;

	return false;	
}

bool getGlError()
{
    int e = glGetError();
    bool ret = e;
    while (e)
    {
        switch (e)
        {
            case GL_INVALID_ENUM:
                cerr << "GL_INVALID_ENUM\n";
                break;
            case GL_INVALID_VALUE:
                cerr << "GL_INVALID_VALUE\n";
                break;
            case GL_INVALID_OPERATION:
                cerr << "GL_INVALID_OPERATION\n";
                break;
            case GL_STACK_OVERFLOW:
                cerr << "GL_STACK_OVERFLOW\n";
                break;
            case GL_STACK_UNDERFLOW:
                cerr << "GL_STACK_UNDERFLOW\n";
                break;
            case GL_OUT_OF_MEMORY:
                cerr << "GL_OUT_OF_MEMORY\n";
                break;
            default:
                cerr << "Unkown GL error: " << e << endl;
        }
        e = glGetError();
    }
    return ret;
}

DecimalDegrees::DecimalDegrees(double lat, double lon, std::string dblq, bool includeDeg)
    :lat(lat),lon(lon)
{
    int degree, min;
    float sec;

    degree = (int)lat;
    sec = (float) (lat - degree);
    if(sec < 0)
        sec = -sec;
    min = (int)(sec * 60);
    sec = sec * 60;
    sec = (sec - min) * 60;

    std::stringstream lats;
    std::string dir = "N";
    if(degree < 0)
    {
        dir = "S";
        degree = degree*(-1);
    }

    //degree = degree*(-1);
    if (includeDeg)
        lats << std::setfill('0') << std::setw(2) << degree << "`";
    lats << std::setfill('0') << std::setw(2) << min << "'";
    lats << std::setfill('0') << std::setw(5) << std::fixed << std::setprecision(2) << sec << dblq << dir;

    lat_string = lats.str();

    degree = (int)lon;
    sec = (float) (lon - degree);
    if(sec < 0)
        sec = -sec;
    min = (int)(sec * 60);
    sec = sec * 60;
    sec = (sec - min) * 60;

    std::stringstream lons;
    dir = "E";
    if(degree < 0)
    {
        dir = "W";
        degree = degree*(-1);
    }

    //degree = degree*(-1);
    if (includeDeg)
        lons << std::setfill('0') << setw(3) << degree << "`";
    lons << std::setfill('0') << setw(2) << min << "'";
    lons << std::setfill('0') << setw(5) << std::fixed << setprecision(2) << sec << dblq << dir;

    lon_string = lons.str();
}
