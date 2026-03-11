// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ANGLE_H
#define GZ4D_ANGLE_H

#include <gz4d/gz4d.h>
#include <cmath>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    struct Degree
    {
        template <typename T> static T period(){return 360.0;}
        template <typename T> static T half_period(){return 180.0;}
    };

    struct Radian
    {
        template<typename T> static T half_period(){return 3.14159265358979323846;}
        template<typename T> static T period(){return half_period<T>()*2.0;}
    };

    struct PositivePeriod
    {
        template<typename T, typename PT> static T fix(T v)
        {
            if(v >= 0.0 && v < PT::template period<T>())
                return v;
            T ret = fmod(v,PT::template period<T>());
            if(ret < 0)
                ret += PT::template period<T>();
            return ret;
        }
    };

    struct ZeroCenteredPeriod
    {
        template<typename T, typename PT> static T fix(T v)
        {
            T half_period = PT::template half_period<T>();
            if(v <= half_period && v > -half_period)
                return v;
            T period = PT::template period<T>();
            T ret = fmod(v,period);
            if(ret > half_period)
                return ret-period;
            if(ret < -half_period)
                return ret+period;
            return ret;
        }
        //void fmod(T v, T period);
    };
    
    template<typename T, typename PT, typename RT = PositivePeriod> class Angle
    {
        T _value;

    public:
        typedef T value_type;
        typedef PT period_type;
        typedef RT range_type;

        Angle():_value(RT::template fix<T, PT>(0.0)){}
        Angle(T v):_value(RT::template fix<T, PT>(v)){}
        Angle(Angle<T,PT,RT> const &a):_value(a._value){}
        template <typename OPT, typename ORT> Angle(Angle<T,OPT,ORT> const &a):_value(RT::template fix<T, PT>(a.normalized()*PT::template period<T>())){}

        T normalized() const{return _value/PT::template period<T>();}
        
        //explicit operator const T () const {return value;}
        T value() const {return _value;}

        bool operator<(Angle<T,PT,RT> o) const {return (o._value-_value > 0.0 && o._value-_value < PT::template half_period<T>()) || o._value-_value < -PT::template half_period<T>();}
        bool operator==(Angle<T,PT,RT> o) const {return _value==o._value;}
        bool operator!=(Angle<T,PT,RT> o) const {return _value!=o._value;}
        bool operator>(Angle<T,PT,RT> o) const {return o<*this;}
        bool operator<=(Angle<T,PT,RT> o) const {return operator<(o)||operator==(o);}
        bool operator>=(Angle<T,PT,RT> o) const {return operator>(o)||operator==(o);}

        Angle<T,PT,RT> const &operator-=(Angle<T,PT,RT> o) {_value = RT::template fix<T, PT>(_value-o._value); return *this;}
        Angle<T,PT,RT> const &operator+=(Angle<T,PT,RT> o) {_value = RT::template fix<T, PT>(_value+o._value); return *this;}
        Angle<T,PT,RT> const &operator*=(T o) {_value = RT::template fix<T, PT>(_value*o); return *this;}
        Angle<T,PT,RT> const &operator/=(T o) {_value = RT::template fix<T, PT>(_value/o); return *this;}

        Angle<T,PT,RT> operator+(Angle<T,PT,RT> o) const {return Angle<T,PT,RT>(*this)+= o;}
        Angle<T,PT,RT> operator-(Angle<T,PT,RT> o) const {return Angle<T,PT,RT>(*this)-= o;}
        Angle<T,PT,RT> operator*(T o) const {return Angle<T,PT,RT>(*this)*= o;}
        Angle<T,PT,RT> operator/(T o) const {return Angle<T,PT,RT>(*this)/= o;}
        
        Angle<T,PT,RT> operator-() const {return Angle<T,PT,RT>(-_value);}

    };

    //namespace angle
    //{
        template <typename T, typename PT, typename RT> Angle<T,PT,RT> interpolate(Angle<T,PT,RT> const &a, Angle<T,PT,RT> const &b, double p=0.5)
        {
            //std::cerr << "interpolate a: " << a << " b: " << b << " p: " << p << std::endl;
            Angle<T,PT,RT> ret;
            if(a<b)
            {
                //std::cerr << "a<b, b-a: " << (b-a) << " (b-a)*p: " << (b-a)*p << std::endl;
                ret =  a+(b-a)*p;
            }
            else
            {
                //std::cerr << "!a<b, a-b: " << (a-b) << " (a-b)*p: " << (a-b)*p << std::endl;
                ret = b+(a-b)*p;
            }
            //std::cerr << ret << std::endl;
            return ret;
        }
    //}
    template <typename T, typename PT, typename RT> inline bool IsNan(Angle<T,PT,RT> const &a)
    {
        return IsNan(a.value());
    }
    
    
    template <typename T, typename RT> inline T sin(Angle<T,Radian,RT> const &a)
    {
        return ::sin(a.value());
    }
    
    template <typename T, typename PT, typename RT> inline T sin(Angle<T,PT,RT> const &a)
    {
        return sin(Angle<T,Radian,RT>(a));
    }

    template <typename T, typename RT> inline T cos(Angle<T,Radian,RT> const &a)
    {
        return ::cos(a.value());
    }

    template <typename T, typename PT, typename RT> inline T cos(Angle<T,PT,RT> const &a)
    {
        return cos(Angle<T,Radian,RT>(a));
    }
    
    template <typename T> inline T cos(T val)
    {
        return ::cos(val);
    }

    template <typename T> inline T sin(T val)
    {
        return ::sin(val);
    }
    
}

#endif