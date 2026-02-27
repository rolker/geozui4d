#include <gz4d/sonar/RDI/FileSource.h>
#include <gz4d/sonar/BottomDetection.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <gz4d/data/binary/Wrapper.h>
#include <gz4d/nav/Track.h>
#include <gz4d/data/NMEA.h>
#include <iostream>
#include <algorithm>

int main(int argc, char *argv[])
{
#ifndef WIN32
    std::ifstream nav(argv[1]);
    if(!nav.is_open())
    {
        std::cerr << "Error opening nav" << std::endl;
        exit(1);
    }
    
    gz4d::nav::Track navTrack;
    gz4d::Time gpstime;
    while(!nav.eof())
    {
        std::string nmeastr;
        nav >> nmeastr;
        try
        {
            gz4d::nmea::Sentence::Ptr nmea(new gz4d::nmea::Sentence(nmeastr));
            if (nmea->IsType("??RMC"))
            {
                gz4d::nmea::RMC rmc(*nmea);
                if(rmc.year > 0 && rmc.month > 0 && rmc.day > 0 && rmc.hour >= 0 && rmc.minute >= 0 && !gz4d::IsNan(rmc.second))
                    gpstime = gz4d::Time(rmc.year,rmc.month,rmc.day,rmc.hour,rmc.minute,rmc.second);
            }
            if (gpstime.valid())
                navTrack.Consume(std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr>(gpstime,nmea));
        }
        catch (...)
        {
        }
    }

    std::ofstream outfile(argv[argc-1],std::ios::out|std::ios::binary);

    for (int i = 2; i < argc-1; ++i)
    {
        gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::RDI::Ping::Ptr>);

        gz4d::RDI::Ping::Ptr pendingPing;

        gz4d::sonar::BottomDetector<gz4d::RDI::Ping>::Ptr bottomDetector(new gz4d::sonar::BottomDetector<gz4d::RDI::Ping>);
        gz4d::Consumer<gz4d::sonar::BottomDetector<gz4d::RDI::Ping>, gz4d::RDI::Ping::Ptr> bottomDetectorThread(bottomDetector);
        bottomDetector->AddOutQueue<gz4d::RDI::Ping::Ptr>(pingQueue);

        
        gz4d::RDI::FileSource fs(argv[i],2009);
        std::cout << argv[i] << std::endl;
        std::cout << fs.GetInfo() << std::endl;

        fs.AddOutQueue(pingQueue);
        //fs.AddOutQueue(bottomDetectorThread.GetInQueue());
        while(fs.ProcessNextEnsemble()||!pingQueue->empty())
        {
            //bottomDetectorThread.WaitEmpty();

            if(!pingQueue->empty())
            {
                pendingPing = pingQueue->pop();
                for(gz4d::RDI::Ping::channels_type::iterator c = pendingPing->GetChannels().begin(); c != pendingPing->GetChannels().end(); ++c)
                {
                    if(!c->Samples().empty())
                    {
                        gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::Samples> samples;
                        samples->geometry.width.x = c->GetGeometry().width[0];
                        samples->geometry.width.y = c->GetGeometry().width[1];
                        samples->geometry.direction.x = c->GetGeometry().direction[0];
                        samples->geometry.direction.y = c->GetGeometry().direction[1];
                        samples->timestamp = pendingPing->Timestamp();
                        samples->channelID = c->ID();
                        samples->offset = c->GetSampleOffset();
                        samples->sampleInterval = c->GetSampleInterval();
                        gz4d::RDI::Ping::channels_type::value_type::iterator end = c->Samples().end();
                        if(c->GetBottomDetection().second)
                            end = c->GetBottomDetection().second;
                        //std::cerr << end - c->Samples().begin() << " samples" << std::endl;
                        samples->samples.size = end-c->Samples().begin();
                        samples.Resize();
                        int outSampleNo = 0;
                        for(gz4d::RDI::Ping::channel_type::sv_iterator s = c->Samples().begin(); s != gz4d::RDI::Ping::channel_type::sv_iterator(end); ++s)
                        {
                            samples->samples.values[outSampleNo] = *s;
                            ++outSampleNo;
                        }
                        samples->samples.size = outSampleNo;
                        samples.Resize();
                        outfile.write(reinterpret_cast<const char *>(&(*samples)),samples->size);
                    }
                }
                gz4d::nav::Fix::Ptr fix = navTrack(pendingPing->Timestamp());
                //std::cout << pendingPing->Timestamp() << "\t" << *fix << std::endl;
                if (!gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->longitude))
                {
                    gz4d::binary::Navigation::Position6DOF::Geographic packet = *fix;
                    packet.heading += 45.0; // HACK - nb150 on Gould has 45deg heading offset.
                    outfile.write(reinterpret_cast<const char *>(&packet),packet.size);
                }
                pendingPing.reset();
            }
        }
    }
#endif
    return 0;
}
