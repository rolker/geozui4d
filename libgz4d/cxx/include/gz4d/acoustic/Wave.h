#ifndef GZ4D_ACOUSTIC_WAVE_H
#define GZ4D_ACOUSTIC_WAVE_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Acoustic.h>
#include "Environment.h"
#include <memory>
#include <gz4d/Phasor.h>
#include <gz4d/Units.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Wave abstract base class.
        /// \ingroup acoustic
        class Wave
        {
            public:
                /// Class representing a wave direction vector.
                class Direction: public Vector<double,3>
                {
                    public:
                        Direction():Vector<double,3>(1.0,0.0,0.0){}
                        Direction(Vector<double,3> const &d):Vector<double,3>(d){}
                        template<typename radType>
                                Direction(radType theta, radType phi):Vector<double,3>(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)){}
                };

                typedef std::shared_ptr<Wave> Ptr;

                Wave(double w):phasor(1.0,0.0),_w(w){}
                Wave(Wave const &wave):phasor(wave.phasor),_w(wave._w),direction(wave.direction){}

                void SetPhasor(Phasor p){phasor = p;}
                Phasor &GetPhasor(){return phasor;}
                Phasor const &GetPhasor()const{return phasor;}

                double GetAmplitude() const {return phasor.Amplitude();}

                /// Returns wave number k in rad/s.
                double Getk(Environment const &env)const{return _w/env.c;}

                void SetAngularFrequency(double w){_w=w;}

                /// Returns wave length in m.
                double getLambda(Environment const &env)const{return 2.0*M_PI*env.c/_w;}

                /// Returns direction vector K
                Direction GetK(Environment const &env)const{ Direction ret(direction); ret*=Getk(env); return ret;}

                /// Returns direction vector normalized to 1.0, so not scaled by k.
                Direction const &GetDirection()const{return direction;}

                void SetDirection(Direction const &d){direction = d;}

            protected:
                Phasor phasor;
                //float k; ///< Spatial frequency in rad/m. 2PI/lambda (lambda is wavelength).
                double _w; ///< angular frequency in rad/s.
                Direction direction;
        };
    }
}

#endif
