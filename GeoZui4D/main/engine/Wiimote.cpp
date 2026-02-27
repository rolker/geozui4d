#include "Wiimote.h"
#ifndef WIN32
#include <iostream>
#include <gz4d/time.h>
#include <gz4d/vector/operations.h>
#include <gz4d/vector/ios.h>

Wiimote::Wiimote()
    :eventQueue(new gz4d::ThreadedQueue<gz4d::Event>)

{
    posable = NULL;
    rotating = false;
    lastTime = gz4d::time::Now();
    wiimoteQueue = wiimoteManager.NewOutQueue();
    wiimoteManager.Scan();
}

void Wiimote::Idle()
{
    if(!wiimoteQueue->empty())
    {
        wiimote = wiimoteQueue->pop();
        wiimote->AddOutQueue(eventQueue);
        wiimote->GetInQueue()->push(gz4d::Event(gz4d::Event::LED,0,1));
    }

    while(!eventQueue->empty())
    {
        gz4d::Event e = eventQueue->pop();
        switch(e.type)
        {
            case gz4d::Event::Axis:
                if(e.id == 0)
                {
                    lastAccel *= 0.9;
                    lastAccel[0] += e.x *0.1;
                    lastAccel[1] += e.y *0.1;
                    lastAccel[2] += e.value *0.1;
                }
                if(e.id == 1)
                {
                    lastNunAccel *= 0.9;
                    lastNunAccel[0] += e.x*0.1;
                    lastNunAccel[1] += e.y*0.1;
                    lastNunAccel[2] += e.value*0.1;
                }
                if(e.id == 2)
                {
                    lastJoystick[0] = e.x;
                    lastJoystick[1] = e.y;
                }
                break;
            case gz4d::Event::Key:
                if (e.id == gz4d::Wiimote::B)
                {
                    if(e.value)
                    {
                        startRotAccel = lastAccel;
                        rotating = true;
                        if (posable)
                        {
                            startOrientation = *posable->getOrientation();
                        }
                    }
                    else
                    {
                        rotating = false;
                    }
                }
                if (e.id == gz4d::Wiimote::Z)
                {
                    if(e.value)
                    {
                        startZoomAccel = lastNunAccel;
                        zooming = true;
                    }
                    else
                    {
                        zooming = false;
                    }
                }
                break;
            default:
                break;
        }
    }

    gz4d::Time now = gz4d::time::Now();
    if (rotating && posable)
    {
        Acceleration rot = cross(normalize(lastAccel),normalize(startRotAccel));
        float dp = asin(rot[0])*360.0/(2.0*3.14);
        float dy = (now-lastTime).asSeconds() * lastAccel[0] * 180.0;
        posable->setOrientation(startOrientation.axis[Pitch]-dp*1.5,posable->getOrientation()->axis[Yaw]+dy);
    }
    if (zooming && posable)
    {
        Acceleration rot = cross(normalize(lastNunAccel),normalize(startZoomAccel));
        //float dp = asin(rot[0]);
        posable->setScale(posable->getScale()* (1.0+rot[0]*(now-lastTime).asSeconds()*2.0));
        std::cerr << posable->getScale()* (1.0+rot[0]*(now-lastTime).asSeconds()*2.0) << std::endl;
    }
    if(lastJoystick[0] > 0.1 || lastJoystick[0] < -0.1 || lastJoystick[1] > 0.1 || lastJoystick[1] < -0.1)
    {
        float heading = posable->getOrientation()->axis[Yaw]*2.0*3.14/360.0;
        float s = posable->getScale();
        s*=2.5;
        float dx = lastJoystick[0]*cos(heading)+lastJoystick[1]*sin(heading);
        float dy = -lastJoystick[0]*sin(heading)+lastJoystick[1]*cos(heading);
        posable->setPosition(posable->getPosition()[0]+(dx/s)*(now-lastTime).asSeconds(),posable->getPosition()[1]+(dy/s)*(now-lastTime).asSeconds(),posable->getPosition()[2]);
    }
    lastTime = now;
}

void Wiimote::SetPosable(PosableStateBase *p)
{
    posable = p;
}

#endif
