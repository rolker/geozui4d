// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SPACEBALL_H
#define GZ4D_SPACEBALL_H

//gz4d_cmake depends spnav

#include <gz4d/gz4d.h>

#include <gz4d/Exception.h>
#include <boost/thread/thread.hpp>
#include <gz4d/thread/Producer.h>
#include <gz4d/Point.h>
#include <gz4d/time.h>
#include <spnav.h>
#include <boost/bind.hpp>


namespace gz4d
{
    struct SpaceballEvent
    {
        time::Point timestamp;
        Point<float> rotation;
        Point<float> translation;
        uint8_t button;
        bool down;
        bool isButton;
    };

    /// Connects to a Spaceball device and returns events.
    /// \ingroup devices
    class Spaceball: public Producer<SpaceballEvent>
    {
        void InputThread()
        {
            spnav_event sev;
            if(spnav_open()!=-1)
            {
                while(!boost::this_thread::interruption_requested())
                {
                    SpaceballEvent e;
                    e.timestamp = time::Now();
                    switch (spnav_poll_event(&sev))
                    {
                        
                        case SPNAV_EVENT_MOTION:
                            e.translation = Point<float>(sev.motion.x/512.0,sev.motion.y/512.0,-sev.motion.z/512.0);
                            e.rotation = Point<float>(sev.motion.rx/512.0,sev.motion.ry/512.0,sev.motion.rz/512.0);
                            e.isButton = false;
                            Send(e);
                            break;
                        case SPNAV_EVENT_BUTTON:
                            e.button = sev.button.bnum;
                            e.isButton = true;
                            e.down = sev.button.press;
                            Send(e);
                            break;
                        default:
                            boost::this_thread::yield();
                    }
                }
                spnav_close();
            }
        }
        
        boost::thread inputThread;            ///< Handle to thread reading the device.
        public:
            Spaceball(){}
            ~Spaceball()
            {
                if(inputThread.joinable())
                {
                    inputThread.interrupt();
                    //inputThread.join();
                }
            }
            
            void Connect()
            {
                inputThread = boost::thread(boost::bind(&Spaceball::InputThread,this));
            }
            
    };
}

#endif
