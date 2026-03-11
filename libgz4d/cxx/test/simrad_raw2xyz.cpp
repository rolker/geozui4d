// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/Simrad/file/FileSource.h>
#include <gz4d/sonar/BottomDetection.h>
#include <gz4d/sonar/TargetDetection.h>
#include <gz4d/nav/Track.h>
#include <gz4d/nav/Synchronizer.h>
#include <gz4d/nav/Integrator.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/data/binary/Wrapper.h>
#include <iostream>
#include <algorithm>
#include <gz4d/time/ios.h>
#include <gz4d/ios.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "usage: simrad_raw2xyz in1.raw [in2.raw ...]" << std::endl;
        exit(1);
    }

    gz4d::Time lastTime(0.0);
    for (int i = 1; i < argc; ++i)
    {
        gz4d::nav::Track::Ptr navTrack(new gz4d::nav::Track);

        gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>);

        gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>::Ptr bottomDetector(new gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>);
        gz4d::Consumer<gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> bottomDetectorThread(bottomDetector);
        bottomDetector->AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(pingQueue);

        gz4d::nav::Track::consumer_type navThread(navTrack);

        gz4d::Simrad::file::Ping::Ptr pendingPing;

        gz4d::time::Duration timeOffset;

        gz4d::Simrad::FileSource fs(argv[i]);
        std::cerr << argv[i] << std::endl;
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(bottomDetectorThread.GetInQueue());
        fs.AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >(navThread.GetInQueue());

        bool ok = fs.ProcessNextDatagram();
        while(ok  || !pingQueue->empty())
        {
            navThread.WaitEmpty();
            bottomDetectorThread.WaitEmpty();

            if(gz4d::IsNan(timeOffset))
            {
                if(!ok)
                    timeOffset = gz4d::time::Duration(0.0);
                else
                    timeOffset = navTrack->GPSOffset();
            }
            if(!pendingPing && !pingQueue->empty())
            {
                pendingPing = pingQueue->pop();
            }
            if(pendingPing && pendingPing->Timestamp() <= navTrack->LastGPSTime())
            {
                gz4d::Simrad::file::Ping::channels_type::iterator c = pendingPing->GetChannels().begin();
                if(!c->Samples().empty())
                {
                    if(c->GetBottomDetection().second)
                    {
                        gz4d::nav::Fix::Ptr fix = (*navTrack)(c->GetTimestamp());
                        gz4d::Simrad::file::Ping::channel_type::range_iterator rbegin = c->GetBottomDetection().first;
                        if(!gz4d::IsNan(fix->longitude) && !gz4d::IsNan(fix->latitude))
                            std::cout << std::fixed << std::setprecision(6) << fix->longitude << "\t" << fix->latitude << "\t" << *rbegin << std::endl;
                    }
                }
                if(pendingPing->Timestamp() > lastTime + gz4d::time::Duration(300))
                {
                    std::cerr << pendingPing->Timestamp() << std::endl;
                    lastTime = pendingPing->Timestamp();
                }
                pendingPing.reset();
            }
            else
            if(!ok)
                break;

            ok = fs.ProcessNextDatagram();
        }
        std::cerr << "done " << i << " of " << argc-1 << std::endl;
    }

    return 0;
}
