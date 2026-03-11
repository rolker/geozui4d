// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/av.h>
#include <gz4d/dsp/SignalDetector.h>

int main(int argc, char* argv[])
{
    gz4d::AVReader avr;
    avr.Open(argv[1]);
    //avr.DumpFormat();

    gz4d::AVReader::AudioStream::output_queue_type aq = avr.GetAudioStreams()[0]->NewOutQueue();
    std::cout << "Time base: " << avr.GetAudioStreams()[0]->time_base << "\t" << 1.0/avr.GetAudioStreams()[0]->time_base <<  std::endl;

    gz4d::SignalDetector sd;
    //sd.init(

    while(avr.ReadFrame())
    {
        if(!aq->empty())
        {
            gz4d::AVReader::AudioStream::output_type ap = aq->pop();
            std::cout << "AudioPacket: " << ap->samples.size() << " samples" << std::endl;
        }
    }
    return 0;
}
