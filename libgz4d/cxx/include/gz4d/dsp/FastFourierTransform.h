// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DSP_FASTFOURIERTRANSFORM_H
#define GZ4D_DSP_FASTFOURIERTRANSFORM_H

#include <gz4d/gz4d.h>
#include "DiscreetFourierTransform.h"

namespace gz4d
{
    
    template<typename T, unsigned N, unsigned S=1, unsigned K=0> struct FastFourierTransform
    {
        typedef T real_type;
        typedef std::complex<T> complex_type;
        enum {size=N};
        
        template<typename OI> static complex_type fft(OI in)
        {
            complex_type factor = expi((K/2)*-2.0*M_PI/T(N));
            complex_type ret = FastFourierTransform<T,N/2,S*2,K>::fft(in)+factor *FastFourierTransform<T,N/2,S*2,K>::fft(in+S);
            return ret;
        }
        
        /// OI - output iterator (ex: vector<T>::iterator)
        /// II - insert_iterator (ex: insert_iterator<vector<T> >
        template <typename OI, typename II> static void transform(OI in, II out)
        {
            complex_type even = FastFourierTransform<T,N/2,S*2,K>::fft(in);
            complex_type odd = FastFourierTransform<T,N/2,S*2,K>::fft(in+1);
            complex_type factor = expi((K/2)*-2.0*M_PI/T(N));
            *out=(even+factor*odd) *(1/sqrt(T(N)));
            FastFourierTransform<T,N,1,K+2>::transform(in,out);
            even = FastFourierTransform<T,N/2,S*2,(N/2)-((K/2)+1)>::fft(in);
            odd = FastFourierTransform<T,N/2,S*2,(N/2)-((K/2)+1)>::fft(in+1);
            factor = -expi((N/2)-((K/2)+1)*-2.0*M_PI/T(N));
            *out=(even+factor*odd) *(1/sqrt(T(N)));
        }
    };
    
    template<typename T, unsigned S, unsigned K> struct FastFourierTransform<T,1,S,K>
    {
        typedef T real_type;
        typedef std::complex<T> complex_type;
        
        template<typename OI> static complex_type fft(OI in)
        {
            return *in;
        }
    };
    
    
    template<typename T, unsigned N> struct FastFourierTransform<T,N,1,N>
    {
        template <typename OI, typename II> static void transform(OI in, II out){}
    };
}

#endif
