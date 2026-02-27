#ifndef GZ4D_TIME_H
#define GZ4D_TIME_H

#include "time/Point.h"
#include "time/Duration.h"
#include "time/Period.h"
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace gz4d
{
    typedef time::Point Time;

    inline void Sleep(time::Duration t)
    {
        boost::this_thread::sleep(boost::posix_time::microseconds(static_cast<int64_t>(t.asSeconds()*1000000.0)));
    }

    inline time::Duration seconds(double s) {return time::Duration(s);}
    inline time::Duration minutes(double m) {return time::Duration(m*60.0);}
    inline time::Duration hours(double h) {return minutes(h*60.0);}
    inline time::Duration days(double d) {return hours(d*24.0);}
    inline time::Duration weeks(double w) {return days(w*7.0);}

    namespace time
    {
        /// Returns the current time.
        /// @return Current time in seconds since the UNIX epoch.
        inline Point Now()
        {
            boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::time_duration ret = now-boost::posix_time::ptime(boost::gregorian::date(1970,1,1));
            return Point(ret.total_seconds()+ret.fractional_seconds()/double(boost::posix_time::time_duration::ticks_per_second()));
        }
        
        /// Convert from Windows NT time stamp format.
        /// low and high form a 64bit int containing
        /// number of 100 nanosecond ticks since Jan 1, 1601
        inline Point FromNT(uint64_t t)
        {
            if(t<EPOCH_AS_NT_TIMESTAMP)
                return Point((EPOCH_AS_NT_TIMESTAMP-t)/-10000000.0);
            return Point((t-EPOCH_AS_NT_TIMESTAMP)/10000000.0);
        }
        
        inline Point FromNT(uint32_t low, uint32_t high)
        {
            return FromNT(static_cast<uint64_t>(high) << 32 | low);
        }
    }
}

#endif
