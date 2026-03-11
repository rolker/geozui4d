// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_BACKSCATTER_H
#define GZ4D_SIMRAD_BACKSCATTER_H

#include <gz4d/sonar/Simrad/file/FileFormat.h>
#include <gz4d/sonar/Simrad/file/Datagram.h>
#include <gz4d/sonar/Channel.h>
#include <gz4d/Point.h>
#include <gz4d/nav/Error.h>

namespace gz4d
{
    namespace Simrad
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
                typedef int16_t result_type;
                inline int16_t operator()(int16_t &value) const {return value;}

                /// Volume scattering
                /// Based on the formula in the EK500 operator's manual
                /// @param Pt Transmitted power in Watts
                /// @param G0 Peak gain in dB re 1
                /// @param r Range in meters
                /// @param l Wavelength in meters
                /// @param c sound speed in m/s
                /// @param tau pulse length in seconds
                /// @param psi Equivalent beam angle in dB re 1 steradian
                /// @param a Absorption coefficient in dB/m
                /// @param Pr Received power in dB re 1 Watt
                /// @param Sa
                /// @return Sv
                inline float CalculateSv(float Pt, float G0, float r, float l, float c, float tau, float psi, float a, float Pr, float Sa) const
                {
                    // 32*pi^2 = 315.8273408348594758027
                    return Pr+ 2*a*r + 10.0*log10((315.8273408348594758027*r*r)/(Pt*l*l*c*tau))-2*G0-psi-2*Sa;
                }

                /// Target Strength
                /// Based on the formula in the EK500 operator's manual
                /// @param Pt Transmitted power in Watts
                /// @param G0 Peak gain in dB re 1
                /// @param r Range in meters
                /// @param l Wavelength in meters
                /// @param c sound speed in m/s
                /// @param psi Equivalent beam angle in dB re 1 steradian
                /// @param a Absorption coefficient in dB/m
                /// @param Pr Received power in dB re 1 Watt
                /// @param Sa
                /// @return TS
                inline float CalculateTS(float Pt, float G0, float r, float l, float c, float a, float Pr, float Sa) const
                {
                    // 16*pi^2 = 157.91367041742973790135
                    return Pr+ 2*a*r + 10.0*log10((157.91367041742973790135*r*r*r*r)/(Pt*l*l))-2*G0-2*Sa;
                }

                inline float CalculatePower(int16_t p) const
                {
                    return p*0.01175898420562426544;
                }

                inline float CalculateTime(int16_t &s) const
                {
                    return (channel->SampleOffset()+(&s-channel->begin()))*channel->SampleInterval();
                }

                inline float CalculateRange(int16_t &s) const
                {
                    return CalculateTime(s)*channel->SoundSpeed()/2.0;
                }

        };


        template <typename T> class Power: public Sample<T>
        {
            public:
                Power(){}
                Power(T* c):Sample<T>(c){}
                Power(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(int16_t &value) const
                {
                    return Sample<T>::CalculatePower(value);
                }
        };

        template <typename T> class Range: public Sample<T>
        {
            public:
                Range(){}
                Range(T* c):Sample<T>(c){}
                Range(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(int16_t &value) const
                {
                    return Sample<T>::CalculateRange(value);
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
                inline Point<float> operator()(int16_t &value) const
                {
                    float r = Sample<T>::CalculateRange(value);
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
                inline nav::Error<float> operator()(int16_t &value) const
                {
                    float r = Sample<T>::CalculateRange(value);
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
                inline float operator()(int16_t &value) const
                {
                    return Sample<T>::CalculateSv(Sample<T>::channel->TransmitPower(), Sample<T>::channel->PeakGain(), Sample<T>::CalculateRange(value), Sample<T>::channel->SoundSpeed()/Sample<T>::channel->Frequency(), Sample<T>::channel->SoundSpeed(), Sample<T>::channel->PulseDuration(), Sample<T>::channel->EquivalentBeamAngle(), Sample<T>::channel->AbsorptionCoefficient(), Sample<T>::CalculatePower(value), Sample<T>::channel->SaCorrection());
                }
        };

        template <typename T> class TS: public Sample<T>
        {
            public:
                TS(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(int16_t &value) const
                {
                    return Sample<T>::CalculateTS(Sample<T>::channel->TransmitPower(), Sample<T>::channel->PeakGain(), Sample<T>::CalculateRange(value), Sample<T>::channel->SoundSpeed()/Sample<T>::channel->Frequency(), Sample<T>::channel->SoundSpeed(), Sample<T>::channel->AbsorptionCoefficient(), Sample<T>::CalculatePower(value), Sample<T>::channel->SaCorrection());
                }
        };

        template <typename T> class Ss: public Sample<T>
        {
            public:
                Ss(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(int16_t &value) const
                {
                    float r = Sample<T>::CalculateRange(value);
                    float TS =  Sample<T>::CalculateTS(Sample<T>::channel->TransmitPower(), Sample<T>::channel->PeakGain(), r, Sample<T>::channel->SoundSpeed()/Sample<T>::channel->Frequency(), Sample<T>::channel->SoundSpeed(), Sample<T>::channel->AbsorptionCoefficient(), Sample<T>::CalculatePower(value), Sample<T>::channel->SaCorrection());
                    float pd = Sample<T>::channel->PulseDuration();
                    float pl = Sample<T>::channel->SoundSpeed()*pd;

                    float farea = Sample<T>::channel->EquivalentBeamAngle()+10.0*log10(r* Sample<T>::channel->BeamGeometry().PulseAreaRatio(r,pl)*Sample<T>::channel->BeamGeometry().AreaRatio());
                    return TS-farea;

                }
        };

        template <typename T> class Time: public Sample<T>
        {
            public:
                Time(Sample<T> const &s):Sample<T>(s){}
                typedef float result_type;
                inline float operator()(int16_t &value) const
                {
                    return Sample<T>::CalculateTime(value);
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
        };

    }
}

#endif
