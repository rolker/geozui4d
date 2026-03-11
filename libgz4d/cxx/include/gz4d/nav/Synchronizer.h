// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NAV_SYNCHRONIZER_H
#define GZ4D_NAV_SYNCHRONIZER_H

#include <gz4d/nav/Track.h>
#include <gz4d/data/binary/Packet.h>

namespace gz4d
{
    namespace nav
    {
        class Synchronizer: public Producer<Fix::Ptr>
        {
            Track::Ptr track;
            time::Duration interval;
            time::Duration timeout;
            Time lastFixTime;
            public:
                typedef std::shared_ptr<Synchronizer> Ptr;

                Synchronizer(Track::Ptr &track, time::Duration interval = time::Duration(1.0), time::Duration timeout = time::Duration(5.0))
                :track(track),interval(interval),timeout(timeout)
                {
                }
                
                void Consume(Fix::Ptr f)
                {
                    if (!lastFixTime.valid())
                        lastFixTime = Time(f->timestamp - interval);
                    
                    Time nextFixTime = Time(lastFixTime + interval);
                    
                    while (f->timestamp >= nextFixTime)
                    {
                        Fix::Ptr nextFix = (*track)(nextFixTime);
                        if(f->timestamp >= nextFixTime + timeout || nextFix->IsComplete())
                        {
                            if(!Producer<Fix::Ptr>::NoOutputQueue())
                                Send(nextFix);
                            lastFixTime = nextFixTime;
                            nextFixTime = Time(lastFixTime + interval);
                        }
                        else
                            break;
                    }
                }
                
        };
    }
}

#endif
