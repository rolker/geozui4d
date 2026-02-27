#ifndef GZ4D_UTIL_BESSEL_H
#define GZ4D_UTIL_BESSEL_H

#include <gz4d/gz4d.h>
#include <cmath>
//#include <stdint.h>

namespace gz4d
{
    /// Bessel function.
    /// Solutions of the differential equation
    /// \f[ x^{2}\frac{d^{2}y}{dx^{2}}+x\frac{dy}{dx}+(x^{2}-\alpha^{2})y=0 \f]
    /// where the order is an integer \f$ m =  \alpha \f$ can be expressed with the following power series:
    /// \f[ J_{m}(x)=\sum_{n=0}^{\infty}\frac{(-1)^{n}x^{2n+m}}{2^{m+2n}(m+n)!n!} \f]
    template<uint8_t O> double bessel(double x)
    {
        unsigned long mFactorial = 1;
        for(unsigned int i = 2; i <= O; ++i)
            mFactorial *= i;
        
        double j = 1.0;
        
        if(O > 0)
            j = pow(x,(double)O)/(pow((double) 2.0, (double) mFactorial)*mFactorial);
        
        double next = j;
        double factor = -x*x/4.0;
        for(unsigned int n = 1; next != 0.0; ++n)
        {
            next *= factor/(double)((O+n)*n);
            j += next;
        }
        return j;
    }
    
    template<uint8_t O=0> class Bessel
    {
        //unsigned int m; ///< Order of the Bessel function.
        public:
            //Bessel(unsigned int m=0):m(m){}
            double operator()(double x) const
            {
                return bessel<O>(x);
            }
    };
}

#endif
