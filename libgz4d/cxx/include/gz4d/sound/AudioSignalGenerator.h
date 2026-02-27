#ifndef GZ4D_AUDIOSIGNALGENERATOR_H
#define GZ4D_AUDIOSIGNALGENERATOR_H

#include <gz4d/gz4d.h>
#include <gz4d/sound/AudioPacket.h>
#include <gz4d/util/SignalGenerator.h>
#include <gz4d/thread/Producer.h>

namespace gz4d
{
    class AudioSignalGenerator: public SignalGenerator, public Producer<AudioPacket::Ptr>
    {
        public:
            AudioSignalGenerator(double freq=1000.0)
            {
                SetFrequency(freq);
            }
            
            void generatePacket()
            {
                AudioPacket::Ptr p(new AudioPacket);
                p->sample_rate = GetSamplingFrequency();
                p->sample_number = 0;
                FillContainer(p->samples);
                Send(p);
            }
            
    };
}
#endif
