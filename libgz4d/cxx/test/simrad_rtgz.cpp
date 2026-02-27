#include <gz4d/sonar/Simrad/net/Client.h>
#include <gz4d/sonar/Simrad/net/ServerManager.h>
#include <gz4d/sonar/Simrad/net/BottomDetection.h>
#include <gz4d/sonar/Simrad/net/ChannelData.h>
#include <gz4d/data/binary/Wrapper.h>

#include <gz4d/time.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <gz4d/socket/TCPSocket.h>


class NavOut: public gz4d::Consumer<NavOut,gz4d::Time>
{
    gz4d::Simrad::ParameterGroup::Map params;
    gz4d::UDPSocket gzdata;
    gz4d::UDPSocket navdata;

    bool ok;
    boost::mutex okMutex;
    bool nonav;
            
    public:
        NavOut(gz4d::Simrad::ParameterGroup::Map const &p, int nPort, int sPort)
    :params(p),ok(true),nonav(true)
        {
            try
            {
                gzdata.Connect(gz4d::IPSocket::Address("LOOPBACK",sPort));
                navdata.Connect(gz4d::IPSocket::Address("LOOPBACK",nPort));

                params["position"]->AddOutQueue(GetInQueue());
            }
            catch (gz4d::Socket::Exception e)
            {
                std::cerr << "Failed to connect to GeoZui4D: " << e.what() << std::endl;
                boost::mutex::scoped_lock lock(okMutex);
                ok = false;
            }
        }

        bool IsOK()
        {
            boost::mutex::scoped_lock lock(okMutex);
            return ok;
        }

        void Consume(gz4d::Time t)
        {
            double lat = (*params["position"])["lat"]->Get<double>(gz4d::Nan<double>(),t);
            double lon = (*params["position"])["lon"]->Get<double>(gz4d::Nan<double>(),t);

            try
            {

                if(!gz4d::IsNan(lat) && !gz4d::IsNan(lon) && IsOK())
                {
                    gz4d::binary::Navigation::Position6DOF::Geographic packet;
                    packet.timestamp = t;
                    packet.latitude = lat;
                    packet.longitude = lon;
                    packet.z = 0.0;
    
                    double heading = 0.0;
                    if ((*params["heading"])["heading"])
                        heading = (*params["heading"])["heading"]->Get<double>(heading);
    
                    packet.heading = heading;
                    packet.pitch = 0.0;
                    packet.roll = 0.0;
                    gzdata.Send(reinterpret_cast<char const *>(&packet), packet.size);
                    navdata.Send(reinterpret_cast<char const *>(&packet), packet.size);
                }
            }
            catch (gz4d::Socket::Exception e)
            {
                std::cerr << "NavOut: Failure in connection to GeoZui4D: " << e.what() << std::endl;
                boost::mutex::scoped_lock lock(okMutex);
                ok = false;
            }

        }

};

class SamplesOut: public gz4d::Consumer<SamplesOut, gz4d::Simrad::SampleSet::Ptr>
{
    gz4d::UDPSocket gzdata;
    boost::mutex okMutex;
    bool ok;
    gz4d::Simrad::ParameterGroup::Map params;
    uint16_t channelNumber;

    public:
        SamplesOut(gz4d::Simrad::SampleSubscription::Ptr ss, int sPort, uint16_t cNo):ok(false),channelNumber(cNo)
        {
            try
            {
                params = ss->GetPingParameters();

                gzdata.Connect(gz4d::IPSocket::Address("LOOPBACK",sPort));

                ok = true;

                ss->AddOutQueue(GetInQueue());
            }
            catch (gz4d::Socket::Exception e)
            {
                std::cerr << "SamplesOut: Failed to connect to GeoZui4D: " << e.what() << std::endl;
            }
        }

        bool IsOK()
        {
            boost::mutex::scoped_lock lock(okMutex);
            return ok;
        }

        void Consume(gz4d::Simrad::SampleSet::Ptr t)
        {
            if(ok)
            {
                try
                {
                    std::shared_ptr<gz4d::Simrad::net::ChannelData> cd(new gz4d::Simrad::net::ChannelData(params,t,0.0,channelNumber)); 
                    gz4d::sonar::Channel<gz4d::Simrad::net::ChannelData> c(cd);

                    gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::Samples> samples;
                    samples->geometry.width.x = c.GetGeometry().width[0];
                    samples->geometry.width.y = c.GetGeometry().width[1];
                    samples->geometry.direction.x = c.GetGeometry().direction[0];
                    samples->geometry.direction.y = c.GetGeometry().direction[1];
                    samples->timestamp = c.GetTimestamp();
                    samples->channelID = c.ID();
                    samples->offset = c.GetSampleOffset();
                    samples->sampleInterval = c.GetSampleInterval();
                    gz4d::sonar::Channel<gz4d::Simrad::net::ChannelData>::iterator end = c.Samples().end();
                    if(c.GetBottomDetection().second)
                        end = c.GetBottomDetection().second;

                    samples->samples.size = end-c.Samples().begin();
                    samples.Resize();
                    std::copy(gz4d::sonar::Channel<gz4d::Simrad::net::ChannelData>::sv_iterator(c.Samples().begin()),gz4d::sonar::Channel<gz4d::Simrad::net::ChannelData>::sv_iterator(end),samples->samples.values);
                    samples.SendVia(gzdata);
                }
                catch (gz4d::Socket::Exception e)
                {
                    std::cerr << "SamplesOut: Failure in connection to GeoZui4D: " << e.what() << std::endl;
                    boost::mutex::scoped_lock lock(okMutex);
                    ok = false;
                }
            }
        }
};



namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    int port;
    std::string laddress;
    std::string raddress;
    int targetID;
    bool rt;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("id",po::value<int>(&targetID)->default_value(1),"Server id")
            ("port",po::value<int>(&port)->default_value(37655),"Server UDP port")
            ("laddress",po::value<std::string>(&laddress)->default_value("ANY"),"Local address")
            ("raddress",po::value<std::string>(&raddress)->default_value("BROADCAST"),"Remote address")
            ("rt",po::value<bool>(&rt)->default_value(true),"Use as RT source")
            ("history",po::value<std::vector<std::string> >(),"Files to replay before sending realtime data")
            ;
    
    po::positional_options_description pp;
    pp.add("history", -1);


    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(pp).run(), vm);
        po::notify(vm);
    }
    catch(po::error)
    {
        std::cerr << desc << std::endl;
        exit(1);
    }

    gz4d::TCPSocket gz;
    gz.Connect(gz4d::IPSocket::Address("LOOPBACK",1974));

    std::string ret(1024,0);
    gz.Recv(ret);

    std::string message = "gz newobject Sonar Simrad\n";
    gz.Send(message);

    ret.resize(1024);
    gz.Recv(ret);
    int retcode;
    std::stringstream ss(ret);
    ss >> retcode;
    int oid;
    ss >> oid;

    message = "gz tellObject " + boost::lexical_cast<std::string>(oid) +" connect\n";
    gz.Send(message);
    ret.resize(1024);
    gz.Recv(ret);
    std::stringstream p(ret);
    p >> retcode;
    int sPort;
    p >> sPort;
    std::cerr << "Sonar port: " << sPort << std::endl;

    message = "gz newobject BasicVessel Vessel\n";
    gz.Send(message);

    ret.resize(1024);
    gz.Recv(ret);
    std::stringstream vss(ret);
    vss >> retcode;
    int vessel_oid;
    vss >> vessel_oid;

    message = "gz tellObject "+ boost::lexical_cast<std::string>(vessel_oid) + " loadobject /home/roland/work/models/ships/LM_Gould/Model/LM_Gould.x3d\n";
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
    std::stringstream np(ret);
    np >> retcode;
    int nPort;
    np >> nPort;
    std::cerr << "Nav Port: " << nPort << std::endl;


    if(rt)
    {
        std::cerr << "RT" << std::endl;
        message = "gz tellObject " + boost::lexical_cast<std::string>(vessel_oid) + " setRT\n";
        gz.Send(message);
        ret.resize(1024);
        gz.Recv(ret);
    }
    else
        std::cerr << "No RT" << std::endl;

    if(vm.count("history"))
    {
        for(std::vector<std::string>::const_iterator h = vm["history"].as<std::vector<std::string> >().begin(); h != vm["history"].as<std::vector<std::string> >().end(); ++h)
        {
            std::cerr << *h << std::endl;
            gz4d::UDPSocket sdata;
            sdata.Connect(gz4d::IPSocket::Address("LOOPBACK",sPort));

            gz4d::UDPSocket ndata;
            ndata.Connect(gz4d::IPSocket::Address("LOOPBACK",nPort));
            

            std::vector<char> data(sizeof(gz4d::binary::Packet));
            std::ifstream in(h->c_str(),std::ios::binary|std::ios::in);
            while(!in.eof())
            {
                in.read(&data.front(),sizeof(gz4d::binary::Packet));
                if(in.eof())
                    break;
                std::size_t size = reinterpret_cast<gz4d::binary::Packet const *>(&data.front())->size;
                data.resize(size);
                in.read(&(data[sizeof(gz4d::binary::Packet)]),size-sizeof(gz4d::binary::Packet));
                if(in.eof())
                    break;
                sdata.Send(data);
                ndata.Send(data);
                gz4d::Sleep(gz4d::seconds(0.01));
            }
        }
    }
    
    {
        gz4d::Simrad::Server s;
        {
            std::cerr << "Launching server manager" << std::endl;
            gz4d::Simrad::ServerManager m(port,laddress,raddress);
    
            gz4d::Time timeout = gz4d::time::Now() + gz4d::time::Duration(5.0);
            while(gz4d::time::Now() < timeout && !s)
            {
                s = m.Get(targetID);
                Sleep(gz4d::seconds(.1));
            }
        }
        if(s)
        {
            std::cerr << "Connecting to: " << s.String() << std::endl;

            gz4d::Simrad::Client c(s);
            c.Connect();

            NavOut nav(c.GetPlatform()->GetParameters(),nPort,sPort);

            std::vector<gz4d::Simrad::net::Channel::Ptr> channels = c.GetTransducer()->GetChannels();
            std::vector<std::shared_ptr<SamplesOut> > samplesOuts;


            for (std::vector<gz4d::Simrad::net::Channel::Ptr>::iterator c = channels.begin(); c != channels.end(); ++c)
            {
                float range = 100.0;
                std::string name = (*c)->GetName();
                if(name.substr(0,11) == "GPT  38 kHz")
                    range = 1000.0;
                if(name.substr(0,11) == "GPT 120 kHz")
                    range = 250.0;

                std::shared_ptr<SamplesOut> so(new SamplesOut((*c)->GetSamples(range),sPort,c-channels.begin()));
                samplesOuts.push_back(so);
            }
            while(nav.IsOK())
            {
                bool samplesOK = true;
                for (std::vector<std::shared_ptr<SamplesOut> >::iterator s = samplesOuts.begin(); s != samplesOuts.end(); ++s)
                {
                    samplesOK = samplesOK && (*s)->IsOK();
                    if(!samplesOK)
                        break;
                }
                if(!samplesOK)
                    break;
                boost::thread::yield();
            }
        }
        else
            std::cerr << "Server not found" << std::endl;
    }
    return 0;
}
