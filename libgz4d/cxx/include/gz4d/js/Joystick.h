// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_JS_JOYSTICK_H
#define GZ4D_JS_JOYSTICK_H

//gz4d_cmake depends js

#include <gz4d/gz4d.h>
#include <boost/thread/thread.hpp>
#include <gz4d/thread/Producer.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <gz4d/time.h>


namespace gz4d
{
    struct JoystickEvent
    {
        time::Point timestamp;
        uint8_t source;
        float position;
        bool down;
        bool isAxis;
    };

    class Joystick: public Producer<JoystickEvent>
    {
        void InputThread(std::string const &dev_name)
        {
            int fd;
            fd = open(dev_name.c_str(),O_RDONLY|O_NONBLOCK);
            if(fd >= 0)
            {
                while(!boost::this_thread::interruption_requested())
                {
                    js_event e;
                    while(read (fd, &e, sizeof(e)) > 0)
                    {
                        JoystickEvent je;
                        je.timestamp = time::Now();
                        
                        if(e.type & JS_EVENT_AXIS)
                        {
                            je.source = e.number;
                            je.position = e.value/32767.0;
                            je.isAxis = true;
                            
                            Send(je);
                        }
                        else if(e.type & JS_EVENT_BUTTON)
                        {
                            je.source = e.number;
                            je.down = e.value;
                            je.isAxis = false;
                            Send(je);
                        }
                    }
                    boost::this_thread::yield();
                }
                close(fd);
            }
        }

        boost::thread inputThread;            ///< Handle to thread reading the device.

        public:
            Joystick(std::string const &dev_name)
            {
                inputThread = boost::thread(boost::bind(&Joystick::InputThread,this,dev_name));
            }

            ~Joystick()
            {
                if(inputThread.joinable())
                {
                    inputThread.interrupt();
                }
            }
    };
}

#endif
