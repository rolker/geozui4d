// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SPAN_H
#define GZ4D_SPAN_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    template <typename T, typename DT> class Span
    {
        protected:
            Span(){}
            Span(Span const &s):span(s.span){}
            Span(T const &s):span(s){}
        public:
            bool valid() const {return valid(span);}

            bool operator==(Span const &o) const {return o.span == span;}

            DT operator-() const {return DT(-span);}

            Span const &operator+=(Span const &o) {span += o.span; return *this;}
            DT operator+(DT const &o) const {return DT(span+o.span);}

            Span const &operator-=(Span const &o) {span -= o.span; return *this;}
            DT operator-(DT const &o) const {return DT(span-o.span);}

            bool operator!=(Span const &o) const {return span != o.span;}
            bool operator>(Span const &o) const {return span > o.span;}
            bool operator>=(Span const &o) const {return span >= o.span;}
            bool operator<(Span const &o) const {return span < o.span;}
            bool operator<=(Span const &o) const {return span <= o.span;}

            template <typename OT > Span const &operator*=(OT const &d) {span *= d; return *this;}
            template <typename OT > DT operator*(OT const &d) const {return DT(span*d);}

            template <typename OT > Span const &operator/=(OT const &d) {span /= d; return *this;}
            template <typename OT > DT operator/(OT const &d) const {return DT(span/d);}

        protected:
            T span;

    };

    
}

#endif
