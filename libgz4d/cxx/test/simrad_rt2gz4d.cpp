// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

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
#include <gz4d/time/ios.h>

class DataSaver
{
    std::ofstream out;
    boost::mutex outMutex;

    public:
        DataSaver()
        {
            gz4d::Time now = gz4d::time::Now();
            std::stringstream fname;
            fname << now << "_simrad.gz4d";
            out.open(fname.str().c_str(),std::ios::binary|std::ios::out);
        }
        void Save(char const *data, std::size_t size)
        {
            boost::mutex::scoped_lock lock(outMutex);
            out.write(data,size);
            out.flush();
        }
        void Send(std::vector<char> const &data)
        {
            boost::mutex::scoped_lock lock(outMutex);
            out.write(&data.front(),data.size());
            out.flush();
        }
};

class NavOut: public gz4d::Consumer<NavOut,gz4d::Time>
{
    gz4d::Simrad::ParameterGroup::Map params;

    bool ok;
    boost::mutex okMutex;
    std::shared_ptr<DataSaver> saver;
            
    public:
        NavOut(gz4d::Simrad::ParameterGroup::Map const &p, std::shared_ptr<DataSaver> &saver)
    :params(p),ok(true),saver(saver)
        {
            params["position"]->AddOutQueue(GetInQueue());
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
                    saver->Save(reinterpret_cast<char const *>(&packet), packet.size);
                    boost::thread::yield();

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
    boost::mutex okMutex;
    bool ok;
    gz4d::Simrad::ParameterGroup::Map params;
    uint16_t channelNumber;
    std::shared_ptr<DataSaver> saver;

    public:
        SamplesOut(gz4d::Simrad::SampleSubscription::Ptr ss, uint16_t cNo, std::shared_ptr<DataSaver> &saver):ok(false),channelNumber(cNo),saver(saver)
        {
            params = ss->GetPingParameters();
            ok = true;
            ss->AddOutQueue(GetInQueue());
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
                    samples.SendVia(*saver);
                    boost::thread::yield();

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
            ;
    
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);
    }
    catch(po::error)
    {
        std::cerr << desc << std::endl;
        exit(1);
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
                gz4d::Sleep(gz4d::seconds(.1));
            }
        }
        if(s)
        {
            std::cerr << "Connecting to: " << s.String() << std::endl;

            gz4d::Simrad::Client c(s);
            c.Connect();

            std::shared_ptr<DataSaver> saver(new DataSaver);

            NavOut nav(c.GetPlatform()->GetParameters(),saver);

            std::vector<gz4d::Simrad::net::Channel::Ptr> channels = c.GetTransducer()->GetChannels();
            std::vector<std::shared_ptr<SamplesOut> > samplesOuts;


            for (std::vector<gz4d::Simrad::net::Channel::Ptr>::iterator c = channels.begin(); c != channels.end(); ++c)
            {
                float range = 250.0;
//                 std::string name = (*c)->GetName();
//                 if(name.substr(0,11) == "GPT  38 kHz")
//                     range = 1000.0;
//                 if(name.substr(0,11) == "GPT 120 kHz")
//                     range = 250.0;

                std::shared_ptr<SamplesOut> so(new SamplesOut((*c)->GetSamples(range),c-channels.begin(),saver));
                samplesOuts.push_back(so);
            }
/*            while(nav.IsOK())
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
                //boost::thread::yield();
                gz4d::Time::Sleep(1);
            }*/
            char key;
            std::cin >> key;
        }
        else
            std::cerr << "Server not found" << std::endl;
    }
    return 0;
}
