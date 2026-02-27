#include <gz4d/sound/Jack.h>
#include <string>
#include <gz4d/time.h>
#include <gz4d/time/ios.h>

int main(int argc, char * argv[])
{
    {
        gz4d::jack::Client c("gzJack");
        std::cerr << "Sample rate: " << c.GetSampleRate() << std::endl;
        std::cerr << "Buffer size:: " << c.GetBufferSize() << std::endl;
        gz4d::jack::InputPort::Ptr p1 = c.NewInputPort("in_one");
        gz4d::jack::InputPort::Ptr p2 = c.NewInputPort("in_two");
        gz4d::jack::Port::Ptr op1 = c.NewOutputPort("out_one");
        gz4d::jack::Port::Ptr op2 = c.NewOutputPort("out_two");
        c.Activate();
        gz4d::ThreadedQueue<gz4d::AudioPacket::Ptr>::Ptr p1q = p1->NewOutQueue();
        gz4d::Time timeout = gz4d::time::Now()+gz4d::time::Duration(30.0);
        while(gz4d::time::Now() < timeout)
        {
            if(p1q->wait_not_empty(0.5))
            {
                gz4d::AudioPacket::Ptr ap = p1q->pop();
                std::cerr << ap->timestamp;
                for(int i = 0; i < 5; ++i)
                    std::cerr << "\t" << ap->samples[i];
                std::cerr << std::endl;
            }
        }
        std::cerr << "Done!" << std::endl;
    }
    gz4d::Sleep(gz4d::seconds(3));
    std::cerr << "Exiting..." << std::endl;
}
