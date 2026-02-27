#include <gz4d/gz4d.h>
#include <gz4d/sound/AudioSignalGenerator.h>
#include <string>
#include <iostream>
#include <gz4d/sound/Portaudio.h>
#include <gz4d/time.h>
#include <math.h>


int main(int argc, char * argv[])
{
    std::string freq = "400";
    if (argc > 1)
        freq = argv[1];
    gz4d::AudioSignalGenerator sigGen;
    sigGen.SetFrequency(atof(freq.c_str()));
    
    gz4d::portaudio::Context c;
    
    for(int i = 0; i < c.getHostApiCount(); ++i)
    {
        gz4d::portaudio::HostApi ha = c.getHostApi(i);
        std::cout << ha.Info();
        for(int j = 0; j < ha.GetDeviceCount(); ++j)
        {
            gz4d::portaudio::Device d = c.getDevice(ha,j);
            std::cout << "Device: " << d.Info();
        }
    }
    gz4d::portaudio::Device id = c.getDefaultInputDevice();
    std::cout << "\nDefault input device: " << id.Info();
    
    gz4d::portaudio::Device od = c.getDefaultOutputDevice();
    std::cout << "\nDefault output device: " << od.Info();
    
    gz4d::portaudio::Format f = c.getDefaultFormat();
    std::cout << "Format supported? " << f.isSupported() << std::endl;
    
    gz4d::portaudio::Stream s = f.open();
    
    sigGen.SetSamplingFrequency(s.getSampleRate());
    sigGen.AddOutQueue(s.getInQueue());
    sigGen.generatePacket();
    
    gz4d::portaudio::Stream::output_queue_type aq = s.NewOutQueue();
    s.start();
    
    while(true)
    {
        if(s.getInQueue()->empty())
            sigGen.generatePacket();
        if(!aq->empty())
        {
            gz4d::AudioPacket::Ptr ap = aq->pop();
            double sum = 0.0;
            for(std::vector<float>::const_iterator i = ap->samples.begin(); i != ap->samples.end(); ++i)
                sum += (*i)*(*i);
            sum /= double(ap->samples.size());
            sum = sqrt(sum);
            for(int i = 0; i < 100*sum; i++)
                std::cerr << "*";
            std::cerr << std::endl;
        }
    }

    return 0;
}
