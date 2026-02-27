#include <gz4d/gz4d.h>
#include <gz4d/sound/Portaudio.h>
#include <iostream>

int main(int argc, char * argv[])
{
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
    
    return 0;
}
