// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IMU_H
#define GZ4D_IMU_H

#include <gz4d/gz4d.h>
#include <gz4d/time.h>
#include <gz4d/Vector.h>

namespace gz4d
{
    namespace IMU
    {
        struct RawState
        {
            gz4d::Time time;
            uint16_t count;
            Vector<uint16_t,3> mag;
            Vector<uint16_t,3> accel;
            Vector<uint16_t,3> gyro;
        };

        struct State
        {
            gz4d::Time time;
            Vector<float,3> mag;
            Vector<float,3> accel;
            Vector<float,3> gyro;
        };



    }

}

#endif
