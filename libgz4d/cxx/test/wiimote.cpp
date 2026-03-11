// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gz4d.h>
#ifndef WIN32
#include <gz4d/WiimoteManager.h>
#include <gz4d/time.h>
#include <string>
#include <iostream>
#include <gz4d/time/ios.h>

int main(int argc, char *argv[])
{
    gz4d::WiimoteManager wm;
    gz4d::ThreadedQueue<gz4d::Wiimote::Ptr>::Ptr wq(new gz4d::ThreadedQueue<gz4d::Wiimote::Ptr>);
    wm.AddOutQueue(wq);
    wm.Scan();
    gz4d::Wiimote::Ptr wiimote = wq->pop();
    gz4d::ThreadedQueue<gz4d::Event>::Ptr q(new gz4d::ThreadedQueue<gz4d::Event>);
    wiimote->AddOutQueue(q);
    gz4d::ThreadedQueue<gz4d::Event>::Ptr inq = wiimote->GetInQueue();
    //inq->push(gz4d::Event(gz4d::Event::Null,2));
    inq->push(gz4d::Event(gz4d::Event::LED,0,6));
    bool done = false;
    while(!done)
    {
        if(!q->empty())
        {
            gz4d::Event e = q->pop();
            if(e.type != gz4d::Event::Axis)
                std::cout << e.timestamp << "\t" << e.type << ": " << e.id << "\tx: " << e.x << "\ty: " << e.y << "\tValue: " << e.value << std::endl;
            else
            {
                std::cout << e.id;
                std::cout.flush();
                //std::cout << "\t" << e.id << " - " << e.x << "\t" << e.y << "\t" << e.value << std::endl;
            }
        }
        else
            boost::thread::yield();
    }
    return 0;
#else
int main(int argc, char *argv[])
{
#endif
    return 1;
}
