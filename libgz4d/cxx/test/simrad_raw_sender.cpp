#include <gz4d/sonar/Simrad/file/FileSource.h>
#include <gz4d/socket/TCPSocket.h>
#include <gz4d/data/binary/Packet.h>
#include <gz4d/socket/UDPSender.h>
#include <gz4d/nav/Track.h>
#include <gz4d/nav/Synchronizer.h>
#include <gz4d/nav/Integrator.h>
#include <gz4d/thread/Consumer.h>
#include <gz4d/sonar/BottomDetection.h>
#include <gz4d/sonar/TargetDetection.h>
#include <gz4d/sonar/Ping.h>
#include <gz4d/sonar/PingSender.h>
#include <gz4d/vector/ios.h>
#include <gz4d/time/ios.h>
#include <gz4d/nav/ios.h>

int main(int argc, char* argv[])
{
    gz4d::TCPSocket gz;

    typedef gz4d::nav::Fix::Ptr NavPacket;
    typedef std::shared_ptr<gz4d::UDPSender<gz4d::nav::Fix::Ptr> > NavSender;
    NavSender navSender;

    typedef gz4d::sonar::BottomDetection::Ptr BottomPacket;
    typedef std::shared_ptr<gz4d::UDPSender<BottomPacket> > BottomSender;
    BottomSender bottomSender;

    typedef gz4d::sonar::TargetDetection::Ptr TargetPacket;
    typedef std::shared_ptr<gz4d::UDPSender<TargetPacket> > TargetSender;
    TargetSender targetSender;

    gz4d::sonar::PingSender<gz4d::Simrad::file::Ping>::Ptr pingSender;

    try
    {
        gz.Connect(gz4d::IPSocket::Address("LOOPBACK",1974));

        std::string ret(1024,0);
        gz.Recv(ret);
                //std::cerr << ret << std::endl;

        std::string message = "gz newobject BasicVessel Vessel\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
        int retcode;
        std::stringstream ss(ret);
        ss >> retcode;
        int vessel_oid;
        ss >> vessel_oid;
        
        message = "gz tellObject "+ boost::lexical_cast<std::string>(vessel_oid) + " loadobject /home/roland/work/models/ships/NancyFoster/NancyFoster.x3d\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);

        message = "gz tellObject "+ boost::lexical_cast<std::string>(vessel_oid) + " projection Geographic\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);

        message = "gz tellObject " + boost::lexical_cast<std::string>(vessel_oid) +" connect\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
        std::stringstream p(ret);
        p >> retcode;
        int port;
        p >> port;
        std::cerr << "Nav Port: " << port << std::endl;

        navSender = NavSender(new gz4d::UDPSender<NavPacket>(gz4d::IPSocket::Address("LOOPBACK",port)));

    }
    catch (gz4d::Socket::Exception e)
    {
        std::cerr << "Failed to connect to GeoZui4D: " << e.what() << std::endl;
    }

    try
    {
        std::string message = "gz newobject XYZStream bottom\n";
        gz.Send(message);

        std::string ret(1024,0);
        gz.Recv(ret);
        int retcode;
        std::stringstream ss(ret);
        ss >> retcode;
        int bottom_oid;
        ss >> bottom_oid;

        message = "gz tellObject " + boost::lexical_cast<std::string>(bottom_oid) +" useAlternateData\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);

        std::string cellsize = "5";
        message = "gz tellObject " + boost::lexical_cast<std::string>(bottom_oid) +" initialize " + cellsize + " swa " + cellsize + "\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);

        message = "gz tellObject " + boost::lexical_cast<std::string>(bottom_oid) +" connect\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
        std::stringstream p(ret);
        p >> retcode;
        int port;
        p >> port;
        std::cerr << "Bottom Port: " << port << std::endl;

        bottomSender = BottomSender(new gz4d::UDPSender<BottomPacket>(gz4d::IPSocket::Address("LOOPBACK",port)));

    }
    catch (gz4d::Socket::Exception e)
    {
        std::cerr << "Failed to connect to GeoZui4D: " << e.what() << std::endl;
    }

    try
    {
        std::string message = "gz newobject Sonar sonar\n";
        gz.Send(message);

        std::string ret(1024,0);
        gz.Recv(ret);
        int retcode;
        std::stringstream ss(ret);
        ss >> retcode;
        int sonar_oid;
        ss >> sonar_oid;

        message = "gz tellObject " + boost::lexical_cast<std::string>(sonar_oid) +" connect\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
        std::stringstream p(ret);
        p >> retcode;
        int port;
        p >> port;
        std::cerr << "Sonar Port: " << port << std::endl;
        pingSender = gz4d::sonar::PingSender<gz4d::Simrad::file::Ping>::Ptr(new gz4d::sonar::PingSender<gz4d::Simrad::file::Ping>(gz4d::IPSocket::Address("LOOPBACK",port)));
    }
    catch (gz4d::Socket::Exception e)
    {
        std::cerr << "Failed to connect to GeoZui4D: " << e.what() << std::endl;
    }

    try
    {
        std::string message = "gz newobject FishSchool targets\n";
        gz.Send(message);

        std::string ret(1024,0);
        gz.Recv(ret);
        int retcode;
        std::stringstream ss(ret);
        ss >> retcode;
        int targets_oid;
        ss >> targets_oid;

        message = "gz tellObject " + boost::lexical_cast<std::string>(targets_oid) +" baseSize 5\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);

        message = "gz tellObject " + boost::lexical_cast<std::string>(targets_oid) +" connect\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
        std::stringstream p(ret);
        p >> retcode;
        int port;
        p >> port;
        std::cerr << "Targets Port: " << port << std::endl;
        targetSender = TargetSender(new gz4d::UDPSender<TargetPacket>(gz4d::IPSocket::Address("LOOPBACK",port)));
    }
    catch (gz4d::Socket::Exception e)
    {
        std::cerr << "Failed to connect to GeoZui4D: " << e.what() << std::endl;
    }

    typedef gz4d::nav::Integrator<gz4d::Simrad::file::Ping::Ptr, gz4d::nav::Fix::Ptr, gz4d::nav::Track::Ptr> NavInt;

    gz4d::nav::Track::Ptr navTrack(new gz4d::nav::Track);
    gz4d::nav::Synchronizer::Ptr navSync(new gz4d::nav::Synchronizer(navTrack,gz4d::time::Duration(.5)));
    gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>::Ptr bottomDetector(new gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>);
    gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>::Ptr targetDetector(new gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>(-50.0) );
    NavInt::Ptr navInt(new NavInt(navTrack));

    gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>::Ptr navQueue(new gz4d::ThreadedQueue<gz4d::nav::Fix::Ptr>);
    gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>::Ptr pingQueue(new gz4d::ThreadedQueue<gz4d::Simrad::file::Ping::Ptr>);
    gz4d::ThreadedQueue<BottomPacket>::Ptr bottomQueue(new gz4d::ThreadedQueue<BottomPacket>);
    gz4d::ThreadedQueue<TargetPacket>::Ptr targetQueue(new gz4d::ThreadedQueue<TargetPacket>);

    gz4d::Consumer<gz4d::UDPSender<NavPacket>,NavPacket> navSenderThread(navSender);
    gz4d::Consumer<gz4d::nav::Synchronizer,gz4d::nav::Fix::Ptr> navSyncThread(navSync);
    gz4d::Consumer<gz4d::nav::Track,std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> > navThread(navTrack);
    gz4d::Consumer<gz4d::UDPSender<BottomPacket>,BottomPacket> bottomSenderThread(bottomSender);
    gz4d::Consumer<gz4d::UDPSender<TargetPacket>,TargetPacket> targetSenderThread(targetSender);
    gz4d::Consumer<gz4d::sonar::BottomDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> bottomDetectorThread(bottomDetector);
    gz4d::Consumer<gz4d::sonar::TargetDetector<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> targetDetectorThread(targetDetector);
    gz4d::Consumer<NavInt, gz4d::Simrad::file::Ping::Ptr> navIntThread(navInt);
    gz4d::Consumer<gz4d::sonar::PingSender<gz4d::Simrad::file::Ping>, gz4d::Simrad::file::Ping::Ptr> pingSenderThread(pingSender);

    navSync->AddOutQueue(navSenderThread.GetInQueue());
    //navSync->AddOutQueue(navQueue);
    navTrack->AddOutQueue(navSyncThread.GetInQueue());
    bottomDetector->AddOutQueue<gz4d::sonar::BottomDetection::Ptr>(bottomSenderThread.GetInQueue());
    //bottomDetector->AddOutQueue<gz4d::sonar::BottomDetection::Ptr>(bottomQueue);
    navInt->AddOutQueue(bottomDetectorThread.GetInQueue());
    navInt->AddOutQueue(pingQueue);
    bottomDetector->AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(pingSenderThread.GetInQueue());
    targetDetector->AddOutQueue<gz4d::sonar::TargetDetection::Ptr>(targetSenderThread.GetInQueue());
    bottomDetector->AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(targetDetectorThread.GetInQueue());

    for (int i = 1; i < argc; ++i)
    {
        gz4d::Simrad::FileSource fs(argv[i]);
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >(navThread.GetInQueue());
        fs.AddOutQueue<gz4d::Simrad::file::Ping::Ptr>(navIntThread.GetInQueue());

        while(fs.ProcessNextDatagram())
        {
            navThread.WaitEmpty();
            navSyncThread.WaitEmpty();
            navIntThread.WaitEmpty();
            bottomDetectorThread.WaitEmpty();

            while(!navQueue->empty())
            {
                gz4d::nav::Fix::Ptr f = navQueue->pop();
                std::cerr << "Sync:\t" << *f << std::endl;
            }

            while(!pingQueue->empty())
            {
                gz4d::Simrad::file::Ping::Ptr p = pingQueue->pop();
                std::cerr << "Ping:\t" << p->Timestamp() << std::endl;
/*                for(gz4d::Simrad::file::Ping::iterator c = p->begin(); c != p->end(); ++c)
                {
                    if(!c->empty())
                    {
                        gz4d::sonar::Channel::Config conf = c->GetConfig();
                        std::cerr << "\tChannel: " << c->ID() << "\tBW: " << conf.beamWidthX << "," << conf.beamWidthY << "\tEBA: " << c->EquivalentBeamAngle() << "\tPL: " << std::setprecision(9) << conf.pulseLength << "\tSI: " << conf.sampleInterval << std::endl;
                    }
                }*/
            }


            while(!bottomQueue->empty())
            {
                BottomPacket b = bottomQueue->pop();
                std::cerr << "Bottom:\t" << b->id << "\t" << b->timestamp << "\t" << "@" << b->position << " var: " << b->variance << std::endl;
            }

        }
    }

    navThread.WaitEmpty();
    navSyncThread.WaitEmpty();
    if(navSender)
        navSenderThread.WaitEmpty();

    navIntThread.WaitEmpty();
    bottomDetectorThread.WaitEmpty();
    if(bottomSender)
        bottomSenderThread.WaitEmpty();
    if(pingSender)
        pingSenderThread.WaitEmpty();
    if(targetSender)
        targetSenderThread.WaitEmpty();
    return 0;
}

