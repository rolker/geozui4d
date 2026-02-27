#include "Spaceball.h"
#ifndef WIN32
#include <gz4d/time.h>

Spaceball::Spaceball()
    :event_queue(new gz4d::ThreadedQueue<gz4d::SpaceballEvent>)
{
    posable = NULL;
    spaceball.AddOutQueue(event_queue);
    spaceball.Connect();
}

void Spaceball::Idle()
{
    while(!event_queue->empty())
    {
        gz4d::SpaceballEvent e = event_queue->pop();
        if(!e.isButton)
        {
            if(lastTime.valid())
            {
                double dt = (e.timestamp-lastTime).asSeconds();
                if (dt<0.25)
                {
                    if (posable)
                    {
                        float x = 0.0;
                        float y = 0.0;
                        float z = 0.0;
                        if (e.rotation[0] < -0.15 || e.rotation[0] > 0.15)
                            x = e.rotation[0];
                        if (e.rotation[1] < -0.15 || e.rotation[1] > 0.15)
                            z = e.translation[1];
                        if (e.rotation[2] < -0.15 || e.rotation[2] > 0.15)
                            y = e.rotation[2];

                        x *= x*x;
                        y *= y*y;
                        z *= z*z;

                        float dp = x*dt*500.0;
                        float dy = z*dt*500.0;
                        float sf = y*dt*5.0;
                        posable->setScale(posable->getScale()*(1.0+sf));
                        posable->setOrientation(posable->getOrientation()->axis[Pitch]+dp,posable->getOrientation()->axis[Yaw]+dy);

                        float heading = posable->getOrientation()->axis[Yaw]*2.0*3.14/360.0;
                        float s = posable->getScale();
                        //s *= 2.5;
                        x = 0.0;
                        y = 0.0;
                        z = 0.0;
                        if (e.translation[0] < -0.15 || e.translation[0] > 0.15)
                            x = e.translation[0];
                        if (e.translation[1] < -0.15 || e.translation[1] > 0.15)
                            z = e.translation[1];
                        if (e.translation[2] < -0.15 || e.translation[2] > 0.15)
                            y = e.translation[2];

                        x *= x*x;
                        y *= y*y;
                        z *= z*z;

                        float dx = x*cos(heading)+y*sin(heading);
                        dy = -x*sin(heading)+y*cos(heading);
                        float dz = z;

                        posable->setPosition(posable->getPosition()[0]+(dx/s)*dt,posable->getPosition()[1]+(dy/s)*dt,posable->getPosition()[2]+(dz/s)*dt);
                        
                        lastTime = e.timestamp;
                    }

                }
            }
            else
            {
                lastTime = e.timestamp;
            }
        }
    }
}

void Spaceball::SetPosable(PosableStateBase *p)
{
    posable = p;
}

#endif
