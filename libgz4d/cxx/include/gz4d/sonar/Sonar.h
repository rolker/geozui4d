// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_SONAR_H
#define GZ4D_SONAR_SONAR_H

#include <gz4d/gz4d.h>

// #ifndef GZ4D_INTERNAL
// #define GZ4D_LIB_NAME gz4d_sonar
// #include <gz4d/config/auto_link.h>
// #endif


namespace gz4d
{
    namespace sonar
    {
        /// Calculate wavelength
        /// @param c sound speed (m/s)
        /// @param f frequency (Hz)
        /// @return wavelength (m)
        inline float Lambda(float c, float f){return c/f;}
    }
}

#endif
