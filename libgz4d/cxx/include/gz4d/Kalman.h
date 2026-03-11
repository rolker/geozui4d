// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_KALMAN_H
#define GZ4D_KALMAN_H

#include <gz4d/gz4d.h>
#include <gz4d/Matrix.h>
#include <gz4d/matrix/operations.h>

namespace gz4d
{
    /// Kalman filter
    /// N is size of internal state vector.
    /// M is size of measurement vector.
    template <typename T, std::size_t N, std::size_t M> class Kalman
    {
        public:
            /// Kalman filter values modified by the algorithm
            struct State
            {
                Vector<T,N> x; ///< State estimate
                Matrix<T,N,N> P; ///< Error covariance matrix
            };

            /// Kalman filter parameters not changed by the algorithm
            struct Params
            {
                Matrix<T,N,N> F; ///< State transition model
                Matrix<T,N,N> Q; ///< process noise covariance, or how much is the state expected to change
                Matrix<T,M,N> H; ///< observation model, maps state space into observed space
                Matrix<T,M,M> R; ///< observation noise covariance matrix
            };

            Kalman(){}
            Kalman(Kalman const &o):state(o.state),params(o.params){}
            Kalman(Vector<T,N> const &x0){state.x = x0;}
            Kalman(Vector<T,N> const &x0, Matrix<T,N,N> const &P0){state.x = x0; state.P = P0;}

            Vector<T,M> operator()(Vector<T,M> const& z) {return operator()(z,params.R);}
            Vector<T,M> operator()(Vector<T,M> const& z, Matrix<T,M,M> const& R);

            State const &GetState() const {return state;}
            State &GetState() {return state;}

            Params const &GetParams() const {return params;}
            Params &GetParams() {return params;}
        protected:
            State state;
            Params params;
    };

    template <typename T, std::size_t N, std::size_t M> inline Vector<T,M> Kalman<T,N,M>::operator()(Vector<T,M> const &z, Matrix<T,M,M> const& R)
    {
        Vector<T,N> xp = params.F*state.x;
        //std::cerr << "\nxp:\t" << xp << std::endl;
        Matrix<T,N,N> Pp = params.F*state.P*transpose(params.F)+params.Q;
        //std::cerr << "Pp:\t" << Pp << std::endl;

        //std::cerr << "z:\t" << z << std::endl;
        Vector<T,M> y = z - (params.H*xp);
        //std::cerr << "y:\t" << y << std::endl;
        Matrix<T,M,M> S = (params.H*Pp*transpose(params.H)) + R;
        //std::cerr << "S:\t" << S << std::endl;
        Matrix<T,N,M> K = Pp*transpose(params.H)*inverse(S);
        //std::cerr << "K:\t" << K << std::endl;
        state.x = xp + (K*y);
        //std::cerr << "x:\t" << state.x << std::endl;
        state.P = (Matrix<T,N,N>::Identity()-K*params.H)*Pp;
        //std::cerr << "P:\t" << state.P << std::endl;
        Vector<T,M> ret = params.H*state.x;
        //std::cerr << "result:\t" << ret << std::endl;
        return ret;
    }
}

#endif
