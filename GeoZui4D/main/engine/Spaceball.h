// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
