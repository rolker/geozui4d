#ifndef OGR_LIMITS_H
#define OGR_LIMITS_H

#include <gz4d/util/Utilities.h>
#include <gz4d/ogr.h>

struct OGR_Limits
{
    double minX, maxX, minY, maxY;
    OGR_Limits():minX(gz4d::Nan<double>()),maxX(gz4d::Nan<double>()),minY(gz4d::Nan<double>()),maxY(gz4d::Nan<double>()){}
    bool operator()(double X, double Y) const;
    bool operator()(gz4d::ogr::Point const &p) const;
    bool operator()(gz4d::Point<double> const &p) const;
    operator bool() const;
};

#endif
