// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_TRANSDUCER_H
#define GZ4D_SIMRAD_TRANSDUCER_H

#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/Channel.h>


namespace gz4d
{
    namespace Simrad
    {
        /// Represents a transducer, which may contain multiple channels.
        class Transducer
        {
            std::map<std::string,net::Channel::Ptr> channels;

            ParameterManager::Ptr parameterManager;
            SubscriptionManager::Ptr subcriptionManager;

            ParameterGroup::Map pingData;

            void Subscribe()
            {
                if(!subscribed)
                {
                    pingData["navigation"]->Add("lat","TransceiverMgr/Latitude",false);
                    pingData["navigation"]->Add("lon","TransceiverMgr/Longitude",false);
                    pingData["navigation"]->Add("heave","TransceiverMgr/Heave");
                    pingData["navigation"]->Add("roll","TransceiverMgr/Roll");
                    pingData["navigation"]->Add("pitch","TransceiverMgr/Pitch");
                    pingData["navigation"]->Add("heading","OwnShip/Heading",false);
                    subscribed = true;
                }
            }
            
            bool subscribed;
            public:
                typedef std::shared_ptr<Transducer> Ptr;

                Transducer(ParameterManager::Ptr &p, SubscriptionManager::Ptr &s)
                :parameterManager(p),subcriptionManager(s),subscribed(false)
                {
                    pingData["offsets"] = ParameterGroup::Ptr(new ParameterGroup(p));
                    pingData["offsets"]->Add("x","OwnShip/TransducerOffsetX");
                    pingData["offsets"]->Add("y","OwnShip/TransducerOffsetY");
                    pingData["offsets"]->Add("z","OwnShip/TransducerOffsetZ");
                    pingData["offsets"]->Add("heading","OwnShip/TransducerAlphaZ");
                    pingData["offsets"]->Add("pitch","OwnShip/TransducerAlphaY");
                    pingData["offsets"]->Add("roll","OwnShip/TransducerAlphaX");
                    
                    pingData["navigation"] = ParameterGroup::Ptr(new ParameterGroup(p));
                    
                    std::vector<std::string> channelList;
                    std::string cs = parameterManager->Get("TransceiverMgr/Channels");
                    boost::algorithm::split(channelList, cs, boost::algorithm::is_any_of(","));
                    
                    for (std::vector<std::string>::iterator c = channelList.begin(); c != channelList.end(); ++c)
                    {
                        channels[*c] = net::Channel::Ptr(new net::Channel(parameterManager, subcriptionManager, pingData, *c));
                    }
                    
                }
                

                std::vector<net::Channel::Ptr> GetChannels()
                {
                    Subscribe();
                    std::vector<gz4d::Simrad::net::Channel::Ptr> ret;
                    for (std::map<std::string,net::Channel::Ptr>::iterator c = channels.begin(); c != channels.end(); ++c)
                        ret.push_back(c->second);
                    return ret;
                }
                

        };
    }
}

#endif
