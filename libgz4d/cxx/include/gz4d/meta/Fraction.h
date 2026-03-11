// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_META_FRACTION_H
#define GZ4D_META_FRACTION_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    template <long N, long D=1> struct Fraction
    {
        enum {numerator = N, denominator = D};
        static double value(){return N/double(D);}
    };
    
    typedef Fraction<355,113> Pi;
    
}    

#endif
