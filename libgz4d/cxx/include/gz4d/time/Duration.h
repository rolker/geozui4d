// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TIME_DURATION_H
#define GZ4D_TIME_DURATION_H

#include <gz4d/Span.h>
#include <gz4d/Exception.h>
#include <boost/math/special_functions.hpp>
#include "TimeConstants.h"

namespace gz4d
{
    namespace time
    {
        class Point;

        class Duration: public Span<double,Duration>
        {
            friend class Point;

            public:
                typedef TypedException<Duration> Exception;

                Duration(){}
                Duration(Duration const &d):Span<double,Duration>(d){}

                /// Initializes to seconds.
                explicit Duration(double d):Span<double,Duration>(d){}

                /// Initializes from string representing duration and unit.
                /// Units are specified as trailing character following the value.
                /// Supporting units with character code: seconds 's', minutes 'm', hours 'h', days 'd' or weeks 'w'.
                /// Examples: "30s", "1.5d", "2w", ".75h".
                Duration(std::string const &ds)
                {
                    if(ds.size() < 2)
                        throw(Exception("Problem converting string to Duration: "+ds));
                    double value = boost::lexical_cast<double>(ds.substr(0,ds.size()-1));
                    switch (*ds.rbegin())
                    {
                        case 's':
                            span = value;
                            break;
                        case 'm':
                            span = value*60.0;
                            break;
                        case 'h':
                            span = value*3600.0;
                            break;
                        case 'd':
                            span = value*SECONDS_PER_DAY;
                            break;
                        case 'w':
                            span = value*SECONDS_PER_WEEK;
                            break;
                        default:
                            throw(Exception("Problem converting string to Duration: "+ds));
                    }
                }
                

                /// Returns duration as seconds.
                double asSeconds() const {return span;}

                /// Returns string representation of duration using largest units where value >= 1 if possible.
                operator std::string() const
                {
                    if(span<60.0 && span > -60.0)
                        return boost::lexical_cast<std::string>(span) + "s";
                    if(span<3600.0 && span > -3600.0)
                        return boost::lexical_cast<std::string>(span/60.0) + "m";
                    if(span<SECONDS_PER_DAY && span > -SECONDS_PER_DAY)
                        return boost::lexical_cast<std::string>(span/3600.0) + "h";
                    if(span<SECONDS_PER_WEEK && span > -SECONDS_PER_WEEK)
                        return boost::lexical_cast<std::string>(span/SECONDS_PER_DAY) + "d";
                    return  boost::lexical_cast<std::string>(span/SECONDS_PER_WEEK) + "w";
                }

        };

        inline double ratio(Duration a, Duration b){return a.asSeconds()/b.asSeconds();}
    }
}

template <typename T > inline gz4d::time::Duration operator*(T a, gz4d::time::Duration const &b) {return b*a;}



#endif
