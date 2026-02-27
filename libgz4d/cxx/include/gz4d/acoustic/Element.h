#ifndef GZ4D_ACOUSTIC_ELEMENT_H
#define GZ4D_ACOUSTIC_ELEMENT_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Wave.h>
#include "Environment.h"
#include <gz4d/Angle.h>

#include <memory>

namespace gz4d
{
    namespace acoustic
    {
        /// Source or receiver with angular dependent response.
        /// Represents a point source with no spacial dimensions.
        /// This is also the base for other Elements and Arrays.
        /// \ingroup acoustic
        class Element
        {
            public:
                typedef Angle<double,Radian,ZeroCenteredPeriod> orientation_type;

                typedef std::shared_ptr<Element> Ptr;

                virtual ~Element(){}

                /// Response for given Wave.
                /// A point source returns 1.0 for all angles.
                virtual double Response(Wave const &wave, Environment const &env)const{return wave.GetAmplitude();}

                /// Beamwidth in given orientation.
                /// @param threshold level in dB.
                /// @return Beamwidth in radians.
                virtual double beamWidth(double w, Environment const &env, double threshold=-3.0, orientation_type direction = 0.0){return M_PI*2.0;}

            protected:
                /// Refines a beamwidth estimate.
                double beamWidth(double w, Environment const &env, double threshold, orientation_type direction, double theta)
                {
                    double target = pow(10.0, threshold/10.0);

                    Wave wave(w);
                    wave.SetDirection(Wave::Direction(theta/2.0,direction.value()));
                    double response = Response(wave, env);
                    while (fabs(target-response)>std::numeric_limits<double>::epsilon()*10.0)
                    {
                        theta *= (1.0-target)/(1.0-response);
                        if (theta > M_PI)
                            theta = M_PI;
                        wave.SetDirection(Wave::Direction(theta/2.0,direction.value()));
                        response = Response(wave, env);
                        if(theta == M_PI && response >= target)
                            return theta;
                    }
                    return theta;
                }
        };
    }
}

#endif
