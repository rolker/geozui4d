#include <gz4d/sonar/Simrad/net/Client.h>
#include <gz4d/sonar/Simrad/net/ServerManager.h>
#include <gz4d/sonar/Simrad/net/BottomDetection.h>
#include <gz4d/time.h>
#include <gz4d/vector/ios.h>
#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <gz4d/time/ios.h>

class NavOut
{
    gz4d::Simrad::ParameterGroup::Map params;
    std::map<std::string, std::shared_ptr<gz4d::ThreadedQueue<gz4d::Time> > > queues;

    public:
        NavOut(gz4d::Simrad::ParameterGroup::Map const &p)
            :params(p)
        {
            std::shared_ptr<gz4d::ThreadedQueue<gz4d::Time> > q = params["position"]->NewOutQueue();
            queues["position"] = q;
            q = params["heading"]->NewOutQueue();
            queues["heading"] = q;
            q = params["attitude"]->NewOutQueue();
            queues["attitude"] = q;
        }

        ~NavOut()
        {
            params.clear();
        }

        void CheckQueues()
        {
            //std::cerr << "CheckQueues" << std::endl;
            if(!queues["position"]->empty())
                Position(queues["position"]->pop());
            if(!queues["heading"]->empty())
                Heading(queues["heading"]->pop());
            if(!queues["attitude"]->empty())
                Attitude(queues["attitude"]->pop());
        }

        void Position(gz4d::Time t)
        {
            std::stringstream s;

            s << t << "\tMyNav:\t" << std::setprecision(10) <<  (*params["position"])["lat"]->Get<double>(gz4d::Nan<double>(),t) << ", " <<  (*params["position"])["lon"]->Get<double>(gz4d::Nan<double>(),t);
            std::cerr << s.str() << std::endl;
        }

        void Heading(gz4d::Time t)
        {
            std::stringstream s;

            s << t << "\tMyNav:\theading: " << (*params["heading"])["heading"]->Get<double>(gz4d::Nan<double>(),t);
            std::cerr << s.str() << std::endl;
        }

        void Attitude(gz4d::Time t)
        {
            std::stringstream s;

            s << t << "\tMyNav:\tp,r: " << (*params["attitude"])["pitch"]->Get<double>(gz4d::Nan<double>(),t) << ", " << (*params["attitude"])["roll"]->Get<double>(gz4d::Nan<double>(),t) << "\theave: " << (*params["attitude"])["heave"]->Get<double>(gz4d::Nan<double>(),t);
            std::cerr << s.str() << std::endl;
        }

};

class TargetOut
{
    std::string label;

    public:
        TargetOut(std::string const &l, gz4d::Simrad::TargetSubscription::Ptr ts):label(l)
        {
            //ts->AddOutQueue(GetInQueue());
        }

        void Consume(gz4d::Simrad::TargetSet t)
        {
            std::stringstream s;
            s << t.time << "\t" << label+": " << t.targets.size() << " targets\n";
            int c = 0;
            for(std::vector<gz4d::Simrad::Target>::iterator i = t.targets.begin(); i != t.targets.end() && i != t.targets.begin()+20;++i)
            {
                s << "  " << i->value << " (" << i->position << ")";
                ++c;
                if(c == 7)
                {
                    s << "\n"; 
                    c = 0;
                }
            }
            std::cerr << s.str() << std::endl;
        }
};

class SamplesOut: gz4d::Consumer<SamplesOut, gz4d::Simrad::SampleSet::Ptr>
{
    std::string label;

    public:
        SamplesOut(std::string const &l, gz4d::Simrad::SampleSubscription::Ptr ss):label(l)
        {
            ss->AddOutQueue(GetInQueue());
        }

        void Consume(gz4d::Simrad::SampleSet t)
        {
            std::stringstream s;
            s << t.time << "\t" << label+": " << t.samples.size() << " samples\n";
            std::cerr << s.str() << std::endl;
        }
};


namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    int port;
    std::string laddress;
    std::string raddress;
    int targetID;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("id",po::value<int>(&targetID)->default_value(1),"Server id")
            ("port",po::value<int>(&port)->default_value(37655),"Server UDP port")
            ("laddress",po::value<std::string>(&laddress)->default_value("ANY"),"Local address")
            ("raddress",po::value<std::string>(&raddress)->default_value("BROADCAST"),"Remote address")
            ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
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
            gz4d::Time timeout = gz4d::time::Now() + gz4d::time::Duration(5.0);
            gz4d::Simrad::ServerManager m(port,laddress,raddress);
            while(gz4d::time::Now() < timeout && !s)
            {
                s = m.Get(targetID);
                gz4d::Sleep(gz4d::seconds(.1));
            }
            std::cerr << "Leaving server manager scope!" << std::endl;
        }
        if(s)
        {
            gz4d::Simrad::Client c(s);
            c.Connect();

            NavOut nav(c.GetPlatform()->GetParameters());

            std::vector<gz4d::Simrad::net::Channel::Ptr> channels = c.GetTransducer()->GetChannels();

            std::vector<std::shared_ptr<TargetOut> > targetOuts;
            std::vector<std::shared_ptr<SamplesOut> > samplesOuts;

            for (std::vector<gz4d::Simrad::net::Channel::Ptr>::iterator c = channels.begin(); c != channels.end(); ++c)
            {
                std::cerr << (*c)->GetName() << std::endl;
                std::shared_ptr<TargetOut> to(new TargetOut( "Target Detection\t"+(*c)->GetName(),(*c)->GetTargetDetection()));
                targetOuts.push_back(to);
                to = std::shared_ptr<TargetOut>(new TargetOut( "Bottom Detection\t"+(*c)->GetName(),(*c)->GetBottomDetection()));
                targetOuts.push_back(to);
                std::shared_ptr<SamplesOut> so(new SamplesOut("Samples\t"+(*c)->GetName(),(*c)->GetSamples()));
                samplesOuts.push_back(so);
            }

            for(int i = 0; i < 75; ++i)
            {
                nav.CheckQueues();
                gz4d::Sleep(gz4d::seconds(.25));
            }

            std::cerr << "Leaving client scope!" << std::endl;
        }
    }
    std::cerr << "Leaving app!" << std::endl;
    return 0;
}
