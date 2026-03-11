// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DOWII_H
#define GZ4D_DOWII_H

#include <gz4d/WiimoteManager.h>
#include <gz4d/Transform.h>

#define SMOOTH_BUFFER 25

namespace gz4d
{
    class Dowii: public gz4d::WiimoteManager
    {
        public:
            Dowii();

            float heading;
            float scale;
            float pitch;
            float roll;

        protected:
            void OnButton(unsigned short id, unsigned short b, bool down);
            void OnAcceleration(unsigned short id, Vector<float,3> a);
            void OnIdle();

            double lastTime;
            Vector<float,3> w[SMOOTH_BUFFER];
            int wi;
            bool zoomingIn;
            bool zoomingOut;
            bool yawing;
    };
};

#endif
