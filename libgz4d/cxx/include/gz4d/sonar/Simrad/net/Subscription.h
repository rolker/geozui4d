#ifndef GZ4D_SIMRAD_SUBSCRIPTION_H
#define GZ4D_SIMRAD_SUBSCRIPTION_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <vector>


namespace gz4d
{
    namespace Simrad
    {
        class Subscription
        {
            Subscription(Subscription const &){}
            Subscription(){}
            protected:
                std::string type;
                std::string channel;
                std::map<std::string,std::string> parameters;
                int id;

                ParameterGroup::Map pingParameters;

                //std::map<gz4d::Time,Navigation> navList;
                //boost::mutex navMutex;


            public:
                typedef std::shared_ptr<Subscription> Ptr;
                typedef std::weak_ptr<Subscription> WeakPtr;

                Subscription(std::string const &t, std::string const &c, ParameterGroup::Map const &pingParams)
                :type(t),channel(c),pingParameters(pingParams)
                {
                }
                
                virtual ~Subscription(){}

                void SetParameter(std::string key, std::string value)
                {
                    parameters[key] = value;
                }
                

                void SetID(int i){id = i;}
                int GetID() const {return id;}

                std::string SubscribeString()
                {
                    std::string ret = type+",ChannelID="+channel;
                    for(std::map<std::string, std::string>::iterator i = parameters.begin(); i!=parameters.end(); ++i)
                    {
                        ret += "," + i->first + "=" + i->second;
                    }
                    return ret;
                }
                
                ParameterGroup::Map const &GetPingParameters() const {return pingParameters;}

                //void AddNav(Navigation &nav);

                virtual void AddData(std::shared_ptr<std::vector<unsigned char> > &data) = 0;
        };


    }
}

#endif

