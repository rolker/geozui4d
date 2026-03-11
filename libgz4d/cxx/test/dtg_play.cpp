// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/dtag.h>
#include <iostream>
#include <gz4d/dtag/MultiAudioStream.h>
#include <cstdlib>
#include <gz4d/sound/Portaudio.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/time/ios.h>

// class OutAudio: public gz4d::AudioDevice
// {
//     gz4d::dtag::MultiAudioStream::Ptr s;
//     std::vector<gz4d::AudioPacket::Ptr> p;
//     int cursor;
// 
//     virtual bool ProduceData(float * buffer,  unsigned long size)
//     {
//         unsigned long i = 0;
//         while(i < size && !done)
//         {
//             if(p.empty())
//             {
//                 p = s->Next();
//                 cursor = 0;
//                 if(p.empty())
//                     done = true;
//                 else
//                     std::cerr << p[0]->timestamp << std::endl;
//             }
//             if(!done)
//             {
//                 bool end = false;
//                 uint8_t cc = p.size();
//                 for(int c = 0; c < cc; ++c)
//                 {
//                     buffer[i*cc+c] = p[c]->samples[cursor];
//                     if(cursor >= p[c]->samples.size()-1)
//                         end = true;
//                 }
//                 ++cursor;
//                 ++i;
//                 if(end)
//                     p.clear();
//             }
//         }
//         if(done)
//             std::cerr << "done!" << std::endl;
//         return true;
//     }
//     public:
//         OutAudio(gz4d::dtag::MultiAudioStream::Ptr s)
//     :s(s),done(false)
//         {
//         }
//         bool done;
// };


void usage()
{
    std::cerr << "Usage: dtgSensor2csv [options] dtagDir\n";
    std::cerr << "   options:\n";
    std::cerr << "     -t -5          (Time offet (hours) from UTC. Defaults to zero. Ex: -t -5 for EST)\n";
    exit(1);
}

int main(int argc, char * argv[])
{
/*
    if (argc < 2)
        usage();

    float timeOffset = 0.0;
    std::vector<std::string> args;

    for(int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "-t")
        {
            if (i < argc-1)
            {
                timeOffset = atof(argv[++i]);
            }
            else
            {
                usage();
            }
        }
        else
            args.push_back(argv[i]);
    }

    if(args.size() == 0)
    {
        usage();
        exit(1);
    }

    for(std::vector<std::string>::iterator i = args.begin(); i != args.end(); i++)
    {
        gz4d::dtag::DTAG dtag(*i);
        gz4d::dtag::MultiAudioStream::Ptr s(new gz4d::dtag::MultiAudioStream);
        dtag.GetStream(*s);
        OutAudio toa(s);
        gz4d::AudioDevice::Stream::DefaultOutputParameters op;
        std::cerr << s->GetChannelCount() << " channels\t" << s->GetSampleRate() << "Hz" << std::endl;
        op.SetChannelCount(s->GetChannelCount());
        gz4d::AudioDevice::Stream outstream;
        outstream.Open(gz4d::AudioDevice::Stream::Format(gz4d::AudioDevice::Stream::Parameters(),op,s->GetSampleRate() ),&toa);
        std::cerr << "Output sample rate: " << outstream.GetSampleRate() << std::endl;
        outstream.Start();
        while(!toa.done)
        {
            std::string in;
            std::cin >> in;
            if(in[0] == 'q')
                break;
            float target = boost::lexical_cast<uint32_t>(in);
            s->Seek(target*1000);
        }
    }*/
    return 0;
}
