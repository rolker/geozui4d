#define GZ4D_INTERNAL
#include <gz4d/x3d/FieldValue.h>

gz4d::x3d::FieldValue::~FieldValue()
{
}

std::istream &gz4d::x3d::operator>>(std::istream &str, Time &t)
{
    double time;
    str >> time;
    t = Time(time);
    return str;
}

std::istream &gz4d::x3d::operator>>(std::istream &str, Matrix<float,3,3> &m)
{
    for(int r = 0; r < 3; ++r)
        for(int c = 0; c < 3; ++c)
            str >> m(r,c);
    return str;
}

std::istream &gz4d::x3d::operator>>(std::istream &str, Matrix<float,4,4> &m)
{
    for(int r = 0; r < 4; ++r)
        for(int c = 0; c < 4; ++c)
            str >> m(r,c);
    return str;
}

std::ostream &gz4d::x3d::operator<<(std::ostream &str, Matrix<float,3,3> &m)
{
    for(int r = 0; r < 3; ++r)
        for(int c = 0; c < 3; ++c)
            str << m(r,c);
        return str;
}

std::ostream &gz4d::x3d::operator<<(std::ostream &str, Matrix<float,4,4> &m)
{
    for(int r = 0; r < 4; ++r)
        for(int c = 0; c < 4; ++c)
            str << m(r,c);
        return str;
}