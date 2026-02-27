#include "objects/ogr/OGR_Limits.h"

bool OGR_Limits::operator()(double x, double y) const
{
    if(!gz4d::IsNan(minX) && x < minX)
        return false;
    if(!gz4d::IsNan(maxX) && x > maxX)
        return false;
    if(!gz4d::IsNan(minY) && y < minY)
        return false;
    if(!gz4d::IsNan(maxY) && y > maxY)
        return false;
    return true;
}

bool OGR_Limits::operator()(const gz4d::Point< double >& p) const
{
    return operator()(p[0],p[1]);
}


bool OGR_Limits::operator()(gz4d::ogr::Point const &p) const
{
    return operator()(p.getX(),p.getY());
}

OGR_Limits::operator bool() const
{
    return !(gz4d::IsNan(minX)&&gz4d::IsNan(maxX)&&gz4d::IsNan(minY)&&gz4d::IsNan(maxY));
}
