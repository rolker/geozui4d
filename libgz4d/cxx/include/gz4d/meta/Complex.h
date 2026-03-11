// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_META_COMPLEX_H
#define GZ4D_META_COMPLEX_H

#include "Fraction.h"

namespace gz4d
{
    template <typename R=Fraction<0>, typename I=Fraction<0> > struct Complex
    {
        typedef R real;
        typedef I imaginary;
    };
    
    template<typename C1, typename C2> struct ComplexMult
    {
        
    };
    
    template<typename X, unsigned n=10> struct Expi
    {
        enum {denom = n*Expi<XN,XD,n-1>::denom,xnum=XN*Expi<XN,XD,n-1>::xnum,xdenom=XD*Expi<XN,XD,n-1>::xdenom};
    };
    
    template<typename X> struct Expi<XN,XD,0>
    {
        enum {denom=1,xnum=1,xdenom=1,pi_num=355,pi_denum=113};
    };
}

#endif  
    