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
