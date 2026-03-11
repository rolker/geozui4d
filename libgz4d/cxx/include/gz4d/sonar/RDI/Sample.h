// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RDI_SAMPLE_H
#define GZ4D_RDI_SAMPLE_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Sonar.h>
#include <gz4d/sonar/BeamGeometry.h>
#include <gz4d/sonar/RDI/FileFormat.h>
#include <gz4d/sonar/RDI/Ensemble.h>
#include <gz4d/Point.h>
#include <gz4d/nav/Error.h>

namespace gz4d
{
    namespace RDI
    {
        template <typename T> class Sample
        {
            protected:
                std::shared_ptr<T> channel;

            public:
                Sample(){}
                Sample(std::shared_ptr<T> &c):channel(c){}
                Sample(Sample<T> const &o):channel(o.channel){}
                operator bool() const {return bool(channel);}
                typedef uint8_t result_type;
                inline uint8_t operator()(uint8_t &value) const {return value;}
                inline float CalculateTime(typename T::iterator const &value) const
                {
                    return channel->SampleOffset()+(value-channel->begin())*channel->SampleInterval();
                }

                inline float CalculateRange(typename T::iterator const &value) const
                {
                    return CalculateTime(value)*channel->SoundSpeed()/2.0;
                }

                inline std::size_t SampleNumber(typename T::iterator const &value) const
                {
                    return value-channel->begin();
                }
        };

        template <typename T> class Range: public Sample<T>
        {
            public:
                Range(){}
                Range(T* c):Sample<T>(c){}
                Range(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                    return Sample<T>::CalculateRange(&value);
                }
        };

        template <typename T> class XYZ: public Sample<T>
        {
            float xr;
            float yr;
            public:
                XYZ(){}
                XYZ(T* c):Sample<T>(c)
                {
                    xr = Radians(c->BeamSteeringX());
                    yr = Radians(c->BeamSteeringY());
                }
                XYZ(Sample<T> const &s):Sample<T>(s)
                {
                    if (Sample<T>::channel)
                    {
                        xr = Radians(Sample<T>::channel->BeamGeometry().direction[0]);
                        yr = Radians(Sample<T>::channel->BeamGeometry().direction[1]);
                    }
                }
                typedef Point<float> result_type;
                inline Point<float> operator()(uint8_t &value) const
                {
                    float r = Sample<T>::CalculateRange(&value);
                    return Point<float>(r * sin(xr), r * sin(yr), -r * cos(xr)*cos(yr));
                }
        };

        template <typename T> class Error: public Sample<T>
        {
            public:
                Error(){}
                Error(T* c):Sample<T>(c){}
                Error(Sample<T> const &s):Sample<T>(s){}
                typedef nav::Error<float> result_type;
                inline nav::Error<float> operator()(uint8_t &value) const
                {
                    float r = Sample<T>::CalculateRange(&value);
                    sonar::BeamGeometry g = Sample<T>::channel->BeamGeometry();
                    float x1 = Radians(g.direction[0] - g.width[0]/2.0);
                    float x2 = Radians(g.direction[0] + g.width[0]/2.0);
                    float xerror = r * fabs(sin(x1)-sin(x2))/2.0;

                    float y1 = Radians(g.direction[1] - g.width[1]/2.0);
                    float y2 = Radians(g.direction[1] - g.width[1]/2.0);
                    float yerror = r * fabs(sin(y1)-sin(y2))/2.0;

                    return nav::Error<float>(xerror,yerror,Nan<float>());
                }
        };

        template <typename T> class Sv: public Sample<T>
        {
            public:
                Sv(T* c):Sample<T>(c){}
                Sv(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                    float ks = 4.17e5;
                    float k2 = 4.3; // rdi fst-003 table 3.1
                    float tx = Sample<T>::channel->Temperature();
                    float k1 = Sample<T>::channel->TransmitPower();
                    float kc = 0.45;
                    float c = Sample<T>::channel->SoundSpeed();
                    float P = Sample<T>::channel->PulseLength();
                    float a = Sample<T>::channel->AbsorptionCoefficient();
                    float R = Sample<T>::CalculateRange(&value);
                    float E = value;
                    float Er = 0.0;
                    return 10.0*log10((4.47e-20*k2*ks*(tx+273)*(pow(10.0,(kc*(E-Er))/10.0)-1.0)*R*R)/(c*P*k1*pow(10.0,(-2.0*a*R)/10.0)));
                }
        };

        template <typename T> class TS: public Sample<T>
        {
            public:
                TS(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                }
        };

        template <typename T> class Ss: public Sample<T>
        {
            public:
                Ss(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
//                     float r = Sample<T>::CalculateRange(&value);
//                     float TS =  Sample<T>::CalculateTS(Sample<T>::channel->TransmitPower(), Sample<T>::channel->PeakGain(), r, Sample<T>::channel->SoundSpeed()/Sample<T>::channel->Frequency(), Sample<T>::channel->SoundSpeed(), Sample<T>::channel->AbsorptionCoefficient(), Sample<T>::CalculatePower(value), Sample<T>::channel->SaCorrection());
//                     float pd = Sample<T>::channel->PulseDuration();
//                     float pl = Sample<T>::channel->SoundSpeed()*pd;
// 
//                     float farea = Sample<T>::channel->EquivalentBeamAngle()+10.0*log10(r* Sample<T>::channel->BeamGeometry().PulseAreaRatio(r,pl)*Sample<T>::channel->BeamGeometry().AreaRatio());
//                     return TS-farea;
                    float ks = 4.17e5;
                    float k2 = 4.3; // rdi fst-003 table 3.1
                    float tx = Sample<T>::channel->Temperature();
                    float k1 = Sample<T>::channel->TransmitPower();
                    float kc = 0.45;
                    float c = Sample<T>::channel->SoundSpeed();
                    float P = Sample<T>::channel->PulseLength();
                    float a = Sample<T>::channel->AbsorptionCoefficient();
                    float R = Sample<T>::CalculateRange(&value);
                    float E = value;
                    float Er = 0.0;
                    return 10.0*log10((4.47e-20*k2*ks*(tx+273)*(pow(10.0,(kc*(E-Er))/10.0)-1.0)*R*R)/(c*P*k1*pow(10.0,(-2.0*a*R)/10.0)));

                }
        };

        template <typename T> class Time: public Sample<T>
        {
            public:
                Time(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                    return Sample<T>::CalculateTime(&value);
                }
        };

        template <typename T> class Velocity: public Sample<T>
        {
            public:
                Velocity(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                    std::size_t i = Sample<T>::SampleNumber(&value);
                    return Sample<T>::channel->GetVelocity(i);
                }
        };

        template <typename T> class PercentGood: public Sample<T>
        {
            public:
                PercentGood(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(uint8_t &value) const
                {
                    std::size_t i = Sample<T>::SampleNumber(&value);
                    return Sample<T>::channel->GetPercentGood(i);
                }
        };

        template <typename T> class Status: public Sample<T>
        {
            public:
                Status(Sample<T> const &s):Sample<T>(s){}
                typedef uint8_t result_type;
                inline uint8_t operator()(uint8_t &value) const
                {
                    std::size_t i = Sample<T>::SampleNumber(&value);
                    return Sample<T>::channel->GetStatus(i);
                }
        };

        template <typename T> struct SampleAdaptors
        {
            typedef Sample<T> raw_type;
            typedef Sv<T> sv_type;
            typedef Range<T> range_type;
            typedef TS<T> ts_type;
            typedef XYZ<T> xyz_type;
            typedef Error<T> error_type;
            typedef Time<T> time_type;
            typedef Ss<T> ss_type;
            typedef Velocity<T> velocity_type;
            typedef PercentGood<T> percent_good_type;
            typedef Status<T> status_type;
        };

    }
}

#endif
