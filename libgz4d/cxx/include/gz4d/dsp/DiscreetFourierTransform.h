#ifndef GZ4D_DSP_DISCREETFOURIERTRANSFORM_H
#define GZ4D_DSP_DISCREETFOURIERTRANSFORM_H

#include <gz4d/gz4d.h>
#include <complex>

namespace gz4d
{
    template<typename T> std::complex<T> expi(T x)
    {
        std::complex<T> ret(::cos(x),::sin(x));
        return ret;
    }


    /// Calculates DFT using basic definition, not optimized!
    /// Used to check FFT implementations or for large and/or
    /// non power of 2 values of N.
    template<typename T> struct DiscreetFourierTransform
    {
        typedef T real_type;
        typedef std::complex<T> complex_type;

        /// OI - output iterator (ex: vector<T>::iterator)
        /// II - insert_iterator (ex: insert_iterator<vector<T> >
        template <typename OI, typename II> static void transform(OI in, II out, std::size_t N)
        {
            for(unsigned k = 0; k < N; ++k)
            {
                complex_type out_val = 0;
                OI in_cursor = in;
                for(unsigned n = 0; n < N; ++n)
                {
                    out_val += (*in_cursor)*expi<T>(n*k*-2.0*M_PI/T(N));
                    ++in_cursor;
                }
                *out=out_val*(T)(1/sqrt(T(N)));
            }
        }
    };
    
    
}



#endif

