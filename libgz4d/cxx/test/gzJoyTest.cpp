#include <gz4d/gz4d.h>

#include <gz4d/js/Joystick.h>
#include <iostream>
#include <gz4d/time.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/vector/ios.h>

int main(int argc, char * argv[])
{
    gz4d::Joystick js(argv[1]);
    gz4d::ThreadedQueue<gz4d::JoystickEvent>::Ptr q = js.NewOutQueue();

    bool done = false;
    while(!done)
    {
        if(!q->empty())
        {
            gz4d::JoystickEvent e = q->pop();
            if (e.isAxis)
                std::cout << e.source << "\t" << e.position << std::endl;
            else
            {
                std::cout << e.source;
                if(e.down)
                    std::cout << "\tDown" << std::endl;
                else
                    std::cout << "\tUp" << std::endl;
            }
        }
        else
            boost::thread::yield();
    }
    return 0;
}
