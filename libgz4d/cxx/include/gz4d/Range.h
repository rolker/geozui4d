// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RANGE_H
#define GZ4D_RANGE_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    struct GreaterThan
    {
        template <typename T> inline bool operator()(T const &a, T const &b) const {return a > b;}
    };

    struct GreaterThanAndEqual
    {
        template <typename T> inline bool operator()(T const &a, T const &b) const {return a >= b;}
    };

    struct LessThan
    {
        template <typename T> inline bool operator()(T const &a, T const &b) const {return a < b;}
    };

    struct LessThanAndEqual
    {
        template <typename T> inline bool operator()(T const &a, T const &b) const {return a <= b;}
    };

    /// \ingroup base
    /// Range of values from a start value to an end value.
    template<typename PT, typename ST, typename LB=GreaterThanAndEqual, typename UB=LessThan> class Range
    {
        protected:
            PT _start;
            PT _finish;

        public:
            typedef PT value_type;

            Range(){}
            Range(PT const &start, PT const &finish):_start(start),_finish(finish){}
            
            inline Range const &operator=(Range const &o) {_start = o._start; _finish=o._finish; return *this;}

            inline bool valid() const {return _start.valid() && _finish.valid() && !(_start == _finish && LB()(_finish,_start) && UB()(_start,_finish));}

            inline PT const &start() const {return _start;}
            inline PT &start() {return _start;}
            inline PT const &finish() const {return _finish;}
            inline PT &finish() {return _finish;}

            inline bool contains(PT const &v) const { return valid() && ( ((_start == _finish) && _start == v) || (_start < _finish && LB()(v,_start) && UB()(v,_finish)) ||(_start > _finish && LB()(_start,v) && UB()(_finish,v)));}

            /// returns true if range represents value going backwards.
            inline bool reversed() const {return valid() && _finish < _start;}

            /// returns true if valid, other is valid and are both going forward or reverse.
            inline bool sameDirection(Range const &o) const {return valid() && o.valid() && reversed() == o.reversed();}

            inline bool overlaps(Range const &o) const { return  (contains(o._start) || contains(o._finish)) && (o.contains(_start) || o.contains(_finish));}

            inline ST span() const {return _finish-_start;}

            /// Interpolates from _start when p = 0 to _finish when p = 1.
            /// Note that values outside the period are returned when 0 > p >= 1.
            inline PT interpolate(double p) const { return _start + (_finish-_start)*p;}

            /// Returns the ratio of time elapsed since _start of period relative to duration of period.
            inline double progress(PT const &v) const { return ratio(v-_start,_finish-_start); }

            /// Returns the overlaping portion if o overlaps, otherwise an invalid range is returned.
            inline Range const &operator&=(Range const &o)
            {
                if(!overlaps(o))
                {
                    _start = PT();
                    _finish = PT();
                }
                else
                {
                    if(reversed())
                    {
                        _start = min(_start,o._start);
                        _finish = max(_finish,o._finish);
                    } 
                    else
                    {
                        _start = max(_start,o._start);
                        _finish =min(_finish,o._finish);
                    }
                }
                return *this;
            }
            inline Range operator&(Range const &o) const {return Range(*this)&=o;}

            /// Returns a period combining both periods if they are valid and going the same direction, otherwise an invalid period is returned.
            Range const &operator|=(Range const &o)
            {
                if(!sameDirection(o))
                {
                    _start = PT();
                    _finish = PT();
                }
                else
                {
                    if(reversed())
                    {
                        _start = max(_start,o._start);
                        _finish = min(_finish,o._finish);
                    }
                    else
                    {
                        _start = min(_start,o._start);
                        _finish = max(_finish,o._finish);
                    }
                }
                return *this;
            }
            Range operator|(Range const &o) const {return Range(_start,_finish)|=o;}
            
            Range const &operator|=(PT const &p) {return *this|=Range(p,p);}
            Range operator|(PT const &p) const {return Range(_start,_finish)|=p;}

            /// Shifts the time period back by duration d.
            Range const &operator<<=(ST s) { _start -= s; _finish -= s; return *this;}
            Range operator<<(ST s) { return Range(_start, _finish)<<= s;}

            /// Shifts the time period forward by duration d.
            Range const &operator>>=(ST s) { _start += s; _finish += s; return *this;}
            Range operator>>(ST s) { return Range(_start, _finish)>>= s;}
    };

    //template <typename RT> inline double progress(typename RT const &v, RT const &r) {return ratio(v-r.start(),r.finish()-r.start());}
}


#endif
