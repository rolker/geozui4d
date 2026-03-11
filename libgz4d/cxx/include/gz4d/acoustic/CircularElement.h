// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ACOUSTIC_CIRCULARELEMENT_H
#define GZ4D_ACOUSTIC_CIRCULARELEMENT_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Element.h>
#include <gz4d/util/Bessel.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Circular element.
        /// \ingroup acoustic
        class CircularElement: public Element
        {
            public:
                CircularElement(double diam=1.0):radius(diam/2.0){}
                CircularElement(CircularElement const &e):radius(e.radius){}

                /// Element response for given sound Wave.
                /// Returns a response value acording to Wave's 
                /// amplitude and direction vector x and y components (Kx and Ky).
                /// Kxy = sqrt(Kx^2+Ky^2)
                /// B(K) = 2 J1(Kxy r)/(Kxy r)
                /// J1 is Bessel function of the first kind with order 1.
                /// J1(z) = sin(z)/z^2 - cos(z)/z
                /// see \ref acoustic for coordinate space details.
                virtual double Response(Wave const &wave, Environment const &env) const
                {
                    Wave::Direction K = wave.GetK(env);
                    double radius_l = radius/wave.getLambda(env);

                    // radius times Kxy (aka: k sin(theta))
                    double rKxy = radius_l*sqrt(K[0]*K[0]+K[1]*K[1]);

                    if(rKxy == 0.0)
                        return wave.GetAmplitude();

                    return wave.GetAmplitude()*2.0*bessel<1>(rKxy)/rKxy;
                }

            protected:
                double radius; ///< radius in meters. (was wavelengths (r/lamda))
        };
    }
}

#endif
