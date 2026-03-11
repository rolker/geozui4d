// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TIME_POINT_H
#define GZ4D_TIME_POINT_H

#include <gz4d/gz4d.h>
#include "Parts.h"
#include <gz4d/util/Utilities.h>
#include <gz4d/Exception.h>
#include "Duration.h"


namespace gz4d
{
    namespace time
    {
        /// Represents an instant in time.
        /// Also allows access to system time.
        class Point
        {
            double time;   ///< Seconds since Jan 1, 1970. NAN if no time assigned.

            public:
                typedef double span_type;

                typedef TypedException<Point> Exception;

                /// Constructor that initialized to the invalid time.
                Point():time(Nan<double>()){}

                /// Constructor that initialized to the supplied seconds since the UNIX epoch.
                explicit Point(double t):time(t){}

                Point(Point const &t):time(t.time){}

                Point(int64_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, double second)
                :time(Parts(year,month,day,hour,minute,second).secondsSinceEpoch())
                {
                }

                Point(int64_t year, double day)
                :time(Parts(year).secondsSinceEpoch()+day*SECONDS_PER_DAY)
                {
                }

#ifndef BOOST_MSVC
                Point(timeval const &t):time(t.tv_sec+t.tv_usec/1000000.0){}
#endif

                Duration secondsSinceEpoch() const
                {
                    return Duration(time);
                }


                bool valid() const {return !boost::math::isnan(time);}

                bool operator>(Point const &o) const {return time > o.time;}
                bool operator<(Point const &o) const {return time < o.time;}
                bool operator>=(Point const &o) const {return time >= o.time;}
                bool operator<=(Point const &o) const {return time <= o.time;}
                bool operator==(Point const &o) const {return time == o.time;}
                bool operator!=(Point const &o) const {return time != o.time;}
                Parts decompose() const {return Parts(time);}

                Point const &operator=(Point o) {time = o.time; return *this;}
                Duration operator-(Point o) const {return Duration(time-o.time);}
                Point operator+(Duration o) const {return Point(time+o.span);}
                Point operator-(Duration o) const {return Point(time-o.span);}
                Point const &operator+=(Duration o){return *this = *this+o;}
                Point const &operator-=(Duration o){return *this = *this-o;}
        };

        inline Point interpolate(Point start, Point end, double p)
        {
            return Point(gz4d::interpolate(start.secondsSinceEpoch().asSeconds(),end.secondsSinceEpoch().asSeconds(),p));
        }

        inline bool valid(Point p){return p.valid();}
        
        inline Point min(Point p1, Point p2) {return std::min(p1,p2);}
        inline Point max(Point p1, Point p2) {return std::max(p1,p2);}
    }
}



#endif
