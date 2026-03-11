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

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "usage: simrad_raw2gz4d sampleSkip in1.raw [in2.raw ...] out.gz4d" << std::endl;
        exit(1);
    }

    std::ofstream outfile(argv[argc-1],std::ios::out|std::ios::binary);
    int sampleSkip = atoi(argv[1]);

    gz4d::Time lastTime(0.0);
    for (int i = 2; i < argc-1; ++i)
    {
        gz4d::nav::Track::Ptr navTrack(new gz4d::nav::Track);
        gz4d::nav::Track::output_queue_type rawNavQueue = navTrack->NewOutQueue();

        gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>);

        gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>::Ptr bottomDetector(new gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>);
        gz4d::Consumer<gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> bottomDetectorThread(bottomDetector);
        bottomDetector->AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(pingQueue);

        gz4d::nav::Track::consumer_type navThread(navTrack);


        navTrack->AddOutQueue(rawNavQueue);

        gz4d::nav::Fix::Ptr pendingNav;
        gz4d::Simrad::file::Ping::Ptr pendingPing;

        gz4d::time::Duration timeOffset;

        gz4d::Simrad::FileSource fs(argv[i]);
        std::cerr << argv[i] << std::endl;
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(bottomDetectorThread.GetInQueue());
        //fs.AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(pingQueue);
        fs.AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >(navThread.GetInQueue());


        bool ok = fs.ProcessNextDatagram();
        while(ok || !rawNavQueue->empty() || !pingQueue->empty())
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
            else
            {
                if(!pendingNav && !rawNavQueue->empty())
                {
                    pendingNav = rawNavQueue->pop();
                }
                if(!pendingPing && !pingQueue->empty())
                {
                    pendingPing = pingQueue->pop();
                }
                if(pendingNav && (!pendingPing || pendingNav->timestamp < pendingPing->Timestamp()))
                {
                    gz4d::binary::Navigation::Position6DOF::Geographic packet = *pendingNav;
                    packet.timestamp = packet.timestamp + timeOffset;
                    outfile.write(reinterpret_cast<const char *>(&packet),packet.size);
                    pendingNav.reset();
                }
                if(pendingPing && (!pendingNav || pendingPing->Timestamp() <= pendingNav->timestamp))
                {
                    for(gz4d::Simrad::file::Ping::channels_type::iterator c = pendingPing->GetChannels().begin(); c != pendingPing->GetChannels().end(); ++c)
                    {
                        if(!c->Samples().empty())
                        {
                            gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::Samples> samples;
                            samples->geometry.width.x = c->GetGeometry().width[0];
                            samples->geometry.width.y = c->GetGeometry().width[1];
                            samples->geometry.direction.x = c->GetGeometry().direction[0];
                            samples->geometry.direction.y = c->GetGeometry().direction[1];
                            samples->timestamp = pendingPing->Timestamp()+timeOffset;
                            samples->channelID = c->ID();
                            samples->offset = c->GetSampleOffset();
                            samples->sampleInterval = c->GetSampleInterval()*sampleSkip;
                            gz4d::Simrad::file::Ping::channels_type::value_type::iterator end = c->Samples().end();
                            if(c->GetBottomDetection().second)
                                end = c->GetBottomDetection().second;
                            //std::cerr << end - c->Samples().begin() << " samples" << std::endl;
                            samples->samples.size = end-c->Samples().begin();
                            samples.Resize();
                            int sampleNo = 0;
                            int outSampleNo = 0;
                            double value = 0.0;
                            for(gz4d::Simrad::file::Ping::channels_type::value_type::sv_iterator s = c->Samples().begin(); s != gz4d::Simrad::file::Ping::channels_type::value_type::sv_iterator(end); ++s)
                            {
                                value += *s;
                                ++sampleNo;
                                if(sampleNo % sampleSkip == 0)
                                {
                                    samples->samples.values[outSampleNo] = value/double(sampleSkip);
                                    ++outSampleNo;
                                    value = 0.0;
                                }
                            }
                            samples->samples.size = outSampleNo;
                            samples.Resize();
                            outfile.write(reinterpret_cast<const char *>(&(*samples)),samples->size);
                        }
                        if(pendingPing->Timestamp() > lastTime + gz4d::time::Duration(300))
                        {
                            std::cerr << pendingPing->Timestamp() << std::endl;
                            lastTime = pendingPing->Timestamp();
                        }

                    }
                    pendingPing.reset();
                }
            }
            ok = fs.ProcessNextDatagram();
        }
    }

    return 0;
}
