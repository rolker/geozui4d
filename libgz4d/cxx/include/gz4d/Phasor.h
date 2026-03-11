// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_PHASOR_H
#define GZ4D_PHASOR_H

#include <gz4d/gz4d.h>
#include <complex>

namespace gz4d
{
    /// Represents an amplitude and phase.
    /// Multiply Phasors to get a phase shift.
    /// Add them to get inteference patterns.
    class Phasor: public std::complex<double>
    {
        public:
            Phasor():std::complex<double>(){}
            Phasor(double r):std::complex<double>(r){}
            Phasor(double r, double i):std::complex<double>(r,i){}
            Phasor(std::complex<double> const &c):std::complex<double>(c){}

            double I(){return real();}
            double Q(){return imag();}
            double Amplitude() const {return abs(*this);}
            double Phase() const {return atan2(imag(),real());}
            void SetAmplitudePhase(double a, double p){*this = std::polar(a,p);}
            using std::complex<double>::operator=;
    };
}

#endif
