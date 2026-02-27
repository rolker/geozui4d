#ifndef GZ4D_TIME_VALUE_H
#define GZ4D_TIME_VALUE_H

#include <gz4d/Value.h>

namespace gz4d
{
    namespace time
    {
        class Point;
        class Duration;

        class TimeValue: public Value<double>
        {
            friend class Point;
            friend class Duration;

            TimeValue(){}
            TimeValue(double t):Value<double>(t){}
            TimeValue(Value<double> const &v):Value<double>(v){}
        };
    }
}

#endif
