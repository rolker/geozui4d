// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gz4d.h>
#include <gz4d/ptu/Ptu.h>
#include <gz4d/js/Joystick.h>

#include <gz4d/thread/ThreadedQueue.h>

int main(int argc, char * argv[])
{
    gz4d::Ptu ptu(argv[1]);
    ptu.PanSpeed(0.0);
    ptu.TiltSpeed(0.0);
    ptu.PanAcceleration(400.0);
    ptu.TiltAcceleration(400.0);

    gz4d::Joystick js(argv[2]);
    gz4d::ThreadedQueue<gz4d::JoystickEvent>::Ptr q = js.NewOutQueue();

    std::cout << "Pan:\trange: " << ptu.GetPanMin() << " - " << ptu.GetPanMax();
    std::cout << "\tspeed range: " << ptu.GetPanMinSpeed() << " - " << ptu.GetPanMaxSpeed() << std::endl;

    std::cout << "Tilt:\trange: " << ptu.GetTiltMin() << " - " << ptu.GetTiltMax();
    std::cout << "\tspeed range: " << ptu.GetTiltMinSpeed() << " - " << ptu.GetTiltMaxSpeed() << std::endl;

    float pan_max_speed = ptu.GetPanMaxSpeed();
    float tilt_max_speed = ptu.GetTiltMaxSpeed();

    ptu.VelocityMode(true);

    bool done = false;
    while(!done)
    {
        bool need_pan = false;
        float pan = 0.0;
        bool need_tilt = false;
        float tilt = 0.0;

        while(!q->empty())
        {
            gz4d::JoystickEvent e = q->pop();
            if (e.isAxis)
            {
                if (e.source == 0)
                {
                    pan = -e.position*pan_max_speed;
                    need_pan = true;
                }
                if (e.source == 1)
                {
                    tilt = e.position*tilt_max_speed;
                    need_tilt = true;
                }
            }
            else
            {
/*                std::cout << jb.source;
                if(jb.direction == gz4d::ui::Down)
                    std::cout << "\tDown" << std::endl;
                else
                    std::cout << "\tUp" << std::endl;*/
            }
        }
        if(need_pan && need_tilt)
            ptu.PanTiltSpeed(pan,tilt);
        else
        {
            if(need_pan)
                ptu.PanSpeed(pan);
            if(need_tilt)
                ptu.TiltSpeed(tilt);
        }
        boost::thread::yield();
    }
    return 0;
}
