#include <gz4d/gz4d.h>
#include <string>
#include <iostream>
#include <gz4d/sound/Jack.h>
#include <gz4d/sound/Portaudio.h>
#include <gz4d/time.h>
#include <math.h>



int main(int argc, char * argv[])
{
    gz4d::jack::Client c("jack2port");
    gz4d::jack::InputPort::Ptr p1 = c.NewInputPort("in");

    gz4d::portaudio::Context pac;
    gz4d::portaudio::Format f = pac.getDefaultFormat();
    f.getInputParameters().setChannelCount(0);
    f.setSampleRate(c.GetSampleRate());
    gz4d::portaudio::Stream s = f.open();
    
    std::cerr << s.getSampleRate() << std::endl;

    p1->AddOutQueue(s.getInQueue());

    std::cin.get();
    
    c.Activate();
    s.start();
    
    return 0;
}
