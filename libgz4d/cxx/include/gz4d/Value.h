// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_VALUE_H
#define GZ4D_VALUE_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{

    template <typename T> struct InvalidDefault
    {
        T operator()(){return Nan<T>();}
    };

    template <typename T, typename DT=InvalidDefault<T> > class Value
    {
        T value;
        public:
            typedef T value_type;

            Value():value(DT()()){}
            explicit Value(T v):value(v){}

            operator T() const {return value;}

            bool operator==(Value const &o) const {return value == o.value;}
            bool operator!=(Value const &o) const {return !operator==(o);}
            bool operator>(Value const &o) const {return value>o.value;}
            bool operator>=(Value const &o) const {return value>=o.value;}
            bool operator<(Value const &o) const {return value<o.value;}
            bool operator<=(Value const &o) const {return value<=o.value;}

            Value const &operator+=(Value const &o) {value += o.value; return *this;}
            Value const &operator-=(Value const &o) {value -= o.value; return *this;}
            Value const &operator*=(Value const &o) {value *= o.value; return *this;}
            Value const &operator/=(Value const &o) {value /= o.value; return *this;}

            Value operator+(Value const &o) const {return Value(value)+=o;}
            Value operator-(Value const &o) const {return Value(value)-=o;}
            Value operator*(Value const &o) const {return Value(value)*=o;}
            Value operator/(Value const &o) const {return Value(value)/=o;}
    };
}

#endif
