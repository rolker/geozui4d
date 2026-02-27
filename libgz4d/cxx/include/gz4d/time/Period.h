#ifndef GZ4D_TIME_PERIOD_H
#define GZ4D_TIME_PERIOD_H

#include <gz4d/gz4d.h>
#include "Point.h"
#include "Duration.h"
#include <gz4d/Range.h>

namespace gz4d
{
    namespace time
    {
        /// Represents a span of time with specific start and stop times.
        /// If end is before start, represents time going backwards.
        class Period: public Range<Point,Duration>
        {
            public:
                /// Constructs an invalid period.
                Period(){}
                Period(Period const &o):Range<Point,Duration>(o){}
                Period(Point start, Point end):Range<Point,Duration>(start,end){}
                /// Constructs a period lasting duration starting at start.
                Period(Point start, Duration duration):Range<Point,Duration>(start,start+duration){}
                /// Constructs a period lasting duration ending at end.
                /// Note that duration should be negative for the period to start before end.
                Period(Duration duration, Point end):Range<Point,Duration>(end+duration,end){}

                Duration duration() const {return span();}

        };
    }
}

#endif
