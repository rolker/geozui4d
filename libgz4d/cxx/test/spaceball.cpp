// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gz4d.h>

#include <gz4d/Spaceball.h>
#include <iostream>
#include <gz4d/time.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/vector/ios.h>

int main(int argc, char * argv[])
{
    gz4d::Spaceball s;
    gz4d::ThreadedQueue<gz4d::SpaceballEvent>::Ptr q = s.NewOutQueue();
    s.Connect();

    bool done = false;
    while(!done)
    {
        if(!q->empty())
        {
            gz4d::SpaceballEvent e = q->pop();
            if (!e.isButton)
            {
                std::cout << e.translation << " rot: " << e.rotation << std::endl;
            }
            else
            {
                std::cout << e.button << "\t";
                if(e.down)
                    std::cout << "Down" << std::endl;
                else
                    std::cout << "Up" << std::endl;
            }
        }
        else
            boost::thread::yield();
    }
    return 0;
}
