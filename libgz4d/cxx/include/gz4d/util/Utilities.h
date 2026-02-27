/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#ifndef GZ4D_UTILITIES_H
#define GZ4D_UTILITIES_H

#include <gz4d/gz4d.h>

#include <boost/math/special_functions.hpp>
#include <boost/algorithm/string.hpp>
#include<string>

namespace gz4d
{
        /// Weighted interpolation between two values.
    /// @param start First value.
    /// @param end Second value.
    /// @param p Proportion of b relative to a.
    /// @return Weighted interpolated value.
    template <typename T> inline T interpolate(T const &start, T const &end, double p)
    {
        return (1.0-p)*start + p*end;
    }



    /// Interpolates two angles, using the shorter distance between them.
    /// With a weight of 0, degree1 is essentialy returned. degree2 is returned when weight is 1.
    /// A weight of 0.5 returns the average of the two angles, or the mid point between them.
    /// @param a First angle.
    /// @param b Second angle.
    /// @param p Proportion of b relative to a.
    /// @return Weighted interpolated value.
    template <typename T> inline T InterpolateDegrees(T a, T b, T p = .5)
    {
        while(a < b-180.0)
            a+=360.0;
        while(a > b+180.0)
            a-=360.0;

        return interpolate(a,b,p);
    }

    template <typename T> inline double ratio(T const &a, T const &b) {return a/b;}

    template <typename T> inline bool IsEven(T i)
    {
        return !(i%2);
    }

    inline bool IsTrue(std::string const &s)
    {
        std::string l = boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(s));
        return (l == "true" || l == "yes" || l == "on" || l == "1" || l == "t" || l == "y");
    }
    
    template<typename T> inline T Nan(){return std::numeric_limits<T>::quiet_NaN();}
    template<typename T> inline bool IsNan(T value){return (boost::math::isnan)(value);}

    /// Used by std::shared_ptr's to hold pointers it shouldn't auto-delete.
    struct NullDeleter
    {
        void operator()(void const *) const {}
    };

    template <typename T> inline T Radians(T degrees) {return degrees*0.01745329251994329577;}
    template <typename T> inline T Degrees(T radians) {return radians*57.2957795130823208768;}
    
    /// Use with lexical_cast to convert hex string to integer type.
    /// From: http://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer
    /// Example: uint32_t value = boost::lexical_cast<HexTo<uint32_t> >("0x2a");
    template <typename ElemT>
    struct HexTo {
        ElemT value;
        operator ElemT() const {return value;}
        friend std::istream& operator>>(std::istream& in, HexTo& out) {
            in >> std::hex >> out.value;
            return in;
        }
    };
};

#endif

