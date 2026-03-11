// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_UNITS_H
#define GZ4D_UNITS_H

#include <gz4d/gz4d.h>
/*#include <boost/units/systems/si.hpp>
#include <boost/units/systems/angle/degrees.hpp>
#include <boost/units/cmath.hpp>*/
//#include <cmath>

namespace gz4d
{

//    namespace units = boost::units;

    //template <typename T> struct Angle : units::quantity<units::si::plane_angle,T>{};
    //template <typename T> struct Length : units::quantity<units::si::length,T>{};

//     static const units::si::plane_angle radian;
//     static const units::degree::plane_angle degree;

//     template<typename type>
//     class Value
//     {
//         protected:
//             type value;
//         public:
//             Value(type v):value(v){}
//             type operator()() const {return value;}
// 
//             Value<type> operator+(Value<type> rvalue) const {return value+rvalue.value;}
//             Value<type> operator-(Value<type> rvalue) const {return value-rvalue.value;}
//             Value<type> operator=(Value<type> rvalue){return value=rvalue.value;}
//             Value<type> operator+=(Value<type> rvalue){return value+=rvalue.value;}
// 
//     };
// 
// 
//     template<typename type>
//     class Meter: public Value<type>
//     {
//         public:
//             Meter():Value<type>(0.0){}
//             Meter(type m):Value<type>(m){}
//     };
// 
// 
//     template<typename type>
//             class Degree;
// 
//     template<typename type>
//     class Radian: public Value<type>
//     {
//         public:
//             Radian():Value<type>(0.0){}
//             Radian(type r):Value<type>(r){}
//             Radian(Radian<type> const &r):Value<type>(r.value){}
//             operator Degree<type>(){return Value<type>::value*360.0/(2.0*M_PI);}
//     };
// 
//     template<typename type>
//     class Degree: public Value<type>
//     {
//         public:
//             Degree():Value<type>(0.0){}
//             Degree(type d):Value<type>(d){}
//             operator Radian<type>(){return Value<type>::value*2.0*M_PI/360.0;}
//     };

    template<typename vType, typename rType> class ValueScaler
    {
        double scale;
        double offset;
        public:
            ValueScaler():scale(1.0),offset(0.0){}
            ValueScaler(ValueScaler const &v):scale(v.scale),offset(v.offset){}
            ValueScaler(double s, double o):scale(s),offset(o){}
            vType Value(rType const &r) const {return r*scale+offset;}
            rType Representation(vType const &v) const {return (v-offset)/scale;}
    };


    template<typename T> class Interval
    {
        T start;
        T end;
        public:
            Interval():start(0.0),end(1.0){}
            Interval(Interval const &i):start(i.start),end(i.end){}
            Interval(T s, T e):start(s),end(e){}

            T GetRange() const {return end-start;}
            T GetStart() const {return start;}
            T GetEnd() const {return end;}
            T Map(double p) const {return start+p*GetRange();}
    };

}

#endif
