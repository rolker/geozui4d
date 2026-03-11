// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/ptu/Ptu.h>
#include <gz4d/time.h>

int main(int argc, char * argv[])
{
    gz4d::Ptu ptu(argv[1]);
    ptu.Tilt(0.0);
    ptu.PanSpeed(200.0);
    ptu.PanAcceleration(400.0);
    ptu.TiltSpeed(200.0);
    ptu.TiltAcceleration(400.0);

    ptu.Pan(90.0);
    Sleep(gz4d::seconds(1.5));
    ptu.Pan(0.0);
    Sleep(gz4d::seconds(1.5));
    ptu.Tilt(25.0);
    Sleep(gz4d::seconds(1.5));
    ptu.Tilt(-25.0);
    Sleep(gz4d::seconds(1));
    ptu.Tilt(0.0);
    Sleep(gz4d::seconds(1));
    ptu.Pan(-90.0);
    Sleep(gz4d::seconds(1.5));
    ptu.Pan(0.0);
    
    return 0;
}