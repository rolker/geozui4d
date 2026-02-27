#ifndef GZ4D_ACOUSTIC_UNIFORMARRAY_H
#define GZ4D_ACOUSTIC_UNIFORMARRAY_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Array.h>
#include "RectangularElement.h"
#include <gz4d/util/Utilities.h>
#include <gz4d/util/Sinc.h>
#include <gz4d/Angle.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Array of uniform elements, uniformly spaced in a rectangular grid.
        /// Inter-Element spacing may be separetly controlled in x and y directions.
        /// \ingroup acoustic
        class UniformArray: public Array
        {
            public:
                typedef Angle<double,Degree,ZeroCenteredPeriod> steering_type;
                
                UniformArray():element(new Element),N(1),M(1),spacing(.5),xSteering(0.0),ySteering(0.0){}
                UniformArray(Element::Ptr &element):element(element),N(1),M(1){}
                UniformArray(UniformArray const &a):element(a.element),N(a.N),M(a.M){}

                void SetNM(unsigned int n, unsigned int m){N=n;M=m;}
                void SetSpacing(Size const &s){spacing = s;}
                void SetElement(Element::Ptr &e){element = e;}
                void SetSteering(double x, double y){xSteering = x; ySteering = y;}
                
                steering_type getSteeringX() const
                {
                    return xSteering;
                }
                steering_type getSteeringY() const
                {
                    return ySteering;
                }


                /// Response of the array from given Wave direction.
                virtual double Response(Wave const &wave, Environment const &env)const
                {
                    Wave::Direction K(wave.GetK(env));
                    // First, get response from the element.
                    double H = element->Response(wave,env);

                    Size spacing_l = spacing/wave.getLambda(env);
                    
                    double k = wave.Getk(env);
                    
                    if (N >1)// && K[0] != 0.0)
                    {
                        
                        double xPhase = ::sin(N*(K[0]-k*sin(xSteering))*spacing_l[0]/2.0)/::sin((K[0]-k*sin(xSteering))*spacing_l[0]/2.0);
                        if (gz4d::IsNan(xPhase))
                            xPhase = wave.GetAmplitude()*N;
                        H *= xPhase/(double)N;
                    }
                    
                    if (M > 1)// && K[1] != 0.0)
                    {
                        double yPhase = ::sin(M*(K[1]-k*sin(ySteering))*spacing_l[1]/2.0)/::sin((K[1]-k*sin(ySteering))*spacing_l[1]/2.0);
                        if (gz4d::IsNan(yPhase))
                            yPhase = wave.GetAmplitude()*M;
                        H *= yPhase/(double)M;
                    }
                    
                    return H;
                    
                }

                virtual double beamWidth(double w, Environment const &env, double threshold=-3.0, orientation_type direction = 0.0)
                {
                    double lengthX = N*spacing[0];
                    double lengthY = M*spacing[1];
                    double theta = RectangularElement(Size(lengthX,lengthY)).beamWidth(w,env,threshold,direction);
                    return Element::beamWidth(w,env,threshold,direction,theta);
                }

                typedef std::shared_ptr<UniformArray> Ptr;


            protected:
                Element::Ptr element;
                unsigned int N;
                unsigned int M;
                Size spacing;
                
                steering_type xSteering, ySteering;
        };
    }
}

#endif
