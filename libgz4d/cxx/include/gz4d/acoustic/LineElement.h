#ifndef GZ4D_ACOUSTIC_LINEELEMENT_H
#define GZ4D_ACOUSTIC_LINEELEMENT_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Element.h>
#include <gz4d/util/Sinc.h>

namespace gz4d
{
    namespace acoustic
    {
        /// 1D line element.
        /// \ingroup acoustic
        class LineElement: public Element
        {
            public:

                LineElement(double l=1.0, orientation_type o=0.0):length(l),orientation(o){}
                LineElement(LineElement const &e):length(e.length),orientation(e.orientation){}

                /// Element response for given Wave.
                /// Returns a response value acording to Wave's 
                /// amplitude and direction vector projected along line orientation.
                /// Defined as A sinc(L/2 k)
                /// see \ref acoustic for coordinate space details.
                virtual double Response(Wave const &wave, Environment const &env) const
                {
                    double length_l = length/wave.getLambda(env);
                    return boost::math::sinc_pi(length_l*(wave.GetK(env)[0]*cos(orientation)+wave.GetK(env)[1]*sin(orientation))/2.0);
                }

                virtual double beamWidth(double w, Environment const &env, float threshold = -3.0, orientation_type direction = 0.0)
                {
                    double target = pow(10.0, threshold/10.0);
                    double x = inv_sinc_pi(target);
                    double eff_length = length*fabs(cos(direction-orientation));
                    double lambda = 2.0*M_PI*env.c/w;
                    double eff_length_l = eff_length/lambda;
                    double k = 2.0*M_PI/lambda;
                    double kx = (2.0*x/eff_length_l)/k;
                    if (kx > 1.0)
                        return M_PI;
                    return 2.0*asin(kx);
                }

                double getLength() const {return length;}

            protected:
                double length; ///< length in meters. (was wavelengths (L/lamda))
                orientation_type orientation; ///< radians, ccw in XY plane. 0 is aligned with X axis.
        };
    }
}

#endif
