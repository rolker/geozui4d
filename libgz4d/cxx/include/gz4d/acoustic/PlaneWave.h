// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ACOUSTIC_PLANEWAVE_H
#define GZ4D_ACOUSTIC_PLANEWAVE_H

#include <gz4d/gz4d.h>
#include <gz4d/acoustic/Wave.h>

namespace gz4d
{
    namespace acoustic
    {
        /// Harmonic plane wave.
        /// \ingroup acoustic
        class PlaneWave: public Wave
        {
            public:
                PlaneWave():direction(0.0,0.0){}
        };
    }
}

#endif
