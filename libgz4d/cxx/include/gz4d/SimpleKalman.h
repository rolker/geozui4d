// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMPLEKALMAN_H
#define GZ4D_SIMPLEKALMAN_H

#include <gz4d/Kalman.h>

namespace gz4d
{
    template <typename T> class SimpleKalman: public Kalman<T,2,1>
    {
        public:
            /// Initialize a single value Kalman filter with estimated starting values.
            /// x0 is estimated initial position
            /// dx0 is estimated initial rate of change (per sample period)
            /// r is variance of the noise of the position measurement
            /// q is variance of the tracked process, or how much do we expecte the state to change
            SimpleKalman(T x0, T dx0, T r, T q);
            SimpleKalman(){}

            /// Add a sample.
            /// z is a measured noisy value.
            /// Returns the estimated value.
            T operator()(T z);

            /// Add a sample with variance.
            /// z is a measured noisy value.
            /// r is variance of measurement.
            /// Returns the estimated value.
            T operator()(T z, T r);
    };

    template <typename T> inline SimpleKalman<T>::SimpleKalman(T x0, T dx0, T r, T q)
    {
        Kalman<T,2,1>::state.x[0] = x0;
        Kalman<T,2,1>::state.x[1] = dx0;

        Kalman<T,2,1>::state.P(0,0) = r;
        Kalman<T,2,1>::state.P(1,1) = r;

        Kalman<T,2,1>::params.F(0,0) = 1;
        Kalman<T,2,1>::params.F(0,1) = 1;
        Kalman<T,2,1>::params.F(1,1) = 1;

        Kalman<T,2,1>::params.Q(0,0) = q;
        Kalman<T,2,1>::params.Q(1,1) = q;

        Kalman<T,2,1>::params.H(0,0) = 1;

        Kalman<T,2,1>::params.R(0,0) = r;
    }

    template <typename T> inline T SimpleKalman<T>::operator()(T z)
    {
        return Kalman<T,2,1>::operator()(Vector<T,1>(z))[0];
    }

    template <typename T> inline T SimpleKalman<T>::operator()(T z, T r)
    {
        return Kalman<T,2,1>::operator()(Vector<T,1>(z),Matrix<T,1,1>(r))[0];
    }
}

#endif
