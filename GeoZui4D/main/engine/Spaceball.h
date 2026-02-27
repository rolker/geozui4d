/* Copyright (c) 2009 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef SPACEBALL_H
#define SPACEBALL_H

#include <gz4d/gz4d.h>

#ifndef WIN32

#include <gz4d/Spaceball.h>
#include "engine/PosableStateBase.h"

class Spaceball
{
    public:
        Spaceball();
        void SetPosable(PosableStateBase *p);
        void Idle();
    private:
        gz4d::Spaceball spaceball;
        gz4d::ThreadedQueue<gz4d::SpaceballEvent>::Ptr event_queue;
        PosableStateBase * posable;
        gz4d::Time lastTime;
};

#endif

#endif
