// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_MATRIX_IOS_H
#define GZ4D_MATRIX_IOS_H

#include <gz4d/Matrix.h>
#include <ostream>

namespace gz4d
{
    template <typename T, std::size_t M, std::size_t N> inline std::ostream &operator<<(std::ostream &s, Matrix<T, M, N> const &m)
    {
        s << "[";
        for (std::size_t r = 0; r < M; ++r)
        {
            s << "[";
            for (std::size_t c = 0; c < N; ++c)
            {
                s << m(r,c);
                if (c < N-1)
                    s << ", ";
            }
            s << "]";
            if (r < M-1)
                s << ", ";
        }
        s << "]";
        return s;
    }
}

#endif
