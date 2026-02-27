/* Copyright (c) 2007 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef WIIMOTE_H
#define WIIMOTE_H

#include <gz4d/gz4d.h>
#ifndef WIN32
#include <gz4d/WiimoteManager.h>
#include "engine/PosableStateBase.h"

class Wiimote
{
    public:
        typedef gz4d::Vector<float,3> Acceleration;
        typedef gz4d::Vector<float,2> Joystick;

        Wiimote();
        void SetPosable(PosableStateBase *p);
        void Idle();
    private:
        PosableStateBase * posable;
        gz4d::Time lastTime;
        bool rotating;
        bool zooming;
        Acceleration lastAccel;
        Acceleration lastNunAccel;
        Joystick lastJoystick;
        Acceleration startRotAccel;
        Acceleration startZoomAccel;
        OrientationInfo startOrientation;
        gz4d::WiimoteManager wiimoteManager;
        gz4d::ThreadedQueue<gz4d::Wiimote::Ptr>::Ptr wiimoteQueue;
        gz4d::ThreadedQueue<gz4d::Event>::Ptr eventQueue;
        gz4d::Wiimote::Ptr wiimote;
};
#endif
#endif
