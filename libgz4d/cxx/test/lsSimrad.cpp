#include <gz4d/sonar/Simrad/file/FileSource.h>
#include <gz4d/sonar/BottomDetection.h>
#include <gz4d/sonar/TargetDetection.h>
#include <gz4d/nav/Track.h>
#include <gz4d/nav/Synchronizer.h>
#include <gz4d/nav/Integrator.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/vector/ios.h>
#include <iostream>
#include <algorithm>
#include <gz4d/time/ios.h>
#include <gz4d/nav/ios.h>

int main(int argc, char *argv[])
{
    typedef gz4d::nav::Integrator<gz4d::Simrad::file::Ping::Ptr, gz4d::nav::Fix::Ptr, gz4d::nav::Track::Ptr> NavInt;
    typedef gz4d::sonar::BottomDetection::Ptr BottomPacket;


    gz4d::nav::Track::Ptr navTrack(new gz4d::nav::Track);
    gz4d::nav::Synchronizer::Ptr navSync(new gz4d::nav::Synchronizer(navTrack,gz4d::time::Duration(.5)));
    gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>::Ptr bottomDetector(new gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>);
    gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>::Ptr bottomDebugDetector(new gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>);
    gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>::Ptr targetDetector(new gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>(-50.0) );
    NavInt::Ptr navInt(new NavInt(navTrack));

    gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>::Ptr navQueue(new gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>);
    gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>::Ptr rawNavQueue(new gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>);
    gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>);
    gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>::Ptr pingDebugQueue(new gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>);gz4d::ThreadedQueue<BottomPacket>::Ptr bottomQueue(new gz4d::ThreadedQueue<BottomPacket>);
    gz4d::ThreadedQueue<gz4d::sonar::TargetDetection::Ptr>::Ptr targetQueue(new gz4d::ThreadedQueue<gz4d::sonar::TargetDetection::Ptr>);

    gz4d::Consumer<gz4d::nav::Synchronizer,gz4d::nav::Fix::Ptr> navSyncThread(navSync);
    gz4d::Consumer<gz4d::nav::Track,std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> > navThread(navTrack);
    gz4d::Consumer<gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> bottomDetectorThread(bottomDetector);
    gz4d::Consumer<gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> targetDetectorThread(targetDetector);
    gz4d::Consumer<NavInt, gz4d::Simrad::file::Ping::Ptr> navIntThread(navInt);

    //navTrack->AddOutQueue(rawNavQueue);
    //navSync->AddOutQueue(navQueue);
    navTrack->AddOutQueue(navSyncThread.GetInQueue());
    bottomDetector->AddOutQueue<gz4d::sonar::BottomDetection::Ptr>(bottomQueue);
    bottomDetector->AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(targetDetectorThread.GetInQueue());
    //targetDetector->AddOutQueue<gz4d::sonar::TargetDetection::Ptr>(targetQueue);
    navInt->AddOutQueue(bottomDetectorThread.GetInQueue());
    navInt->AddOutQueue(pingQueue);
    //navInt->AddOutQueue(pingDebugQueue);

    for (int i = 1; i < argc; ++i)
    {
        gz4d::Simrad::FileSource fs(argv[i]);
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(navIntThread.GetInQueue());
        fs.AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >(navThread.GetInQueue());

        while(fs.ProcessNextDatagram())
        {
            navThread.WaitEmpty();
            navSyncThread.WaitEmpty();
            navIntThread.WaitEmpty();
            bottomDetectorThread.WaitEmpty();

            while(!rawNavQueue->empty())
            {
                gz4d::nav::Fix::Ptr f = rawNavQueue->pop();
                std::cerr << "Async:\t" << *f << std::endl;
            }

            while(!navQueue->empty())
            {
                gz4d::nav::Fix::Ptr f = navQueue->pop();
                std::cerr << "Sync:\t" << *f << std::endl;
            }

            while(!pingQueue->empty())
            {
                gz4d::Simrad::file::Ping::Ptr p = pingQueue->pop();
                std::cerr << "Ping:\t" << p->Timestamp() << std::endl;
                for(gz4d::Simrad::file::Ping::channels_type::iterator c = p->GetChannels().begin(); c != p->GetChannels().end(); ++c)
                {
                    if(!c->Samples().empty())
                    {
                        std::cerr << "\tChannel: " << c->ID() << "\tBW: " << c->GetGeometry().width << "\tPL: " << std::setprecision(9) << c->GetPulse().duration << "\tSI: " << c->GetSampleInterval() << std::endl;
                    }
                }
            }

            while(!bottomQueue->empty())
            {
                gz4d::sonar::BottomDetection::Ptr b = bottomQueue->pop();
                std::cerr << "Bottom:\t" << b->id << "\t" << b->timestamp << "\t" << b->latitude.value() << "," << b->longitude.value() << "\t" << "@" << b->position << std::endl;
            }

            while(!targetQueue->empty())
            {
                gz4d::sonar::TargetDetection::Ptr t = targetQueue->pop();
                std::cerr << "Target:\t" << t->id << "\t" << t->timestamp << "\t" << t->latitude.value() << "," << t->longitude.value() << "\t" << "@" << t->position << std::endl;
            }
            
            while(!pingDebugQueue->empty())
            {
                gz4d::Simrad::file::Ping::Ptr p = pingDebugQueue->pop();
                bottomDebugDetector->Consume(p);
            }

        }
    }


    return 0;
}
