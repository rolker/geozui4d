// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_SUBSCRIPTIONMANAGER_H
#define GZ4D_SIMRAD_SUBSCRIPTIONMANAGER_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Request.h>
#include <gz4d/sonar/Simrad/net/Connection.h>
#include <gz4d/sonar/Simrad/net/SubscriptionPort.h>
#include <gz4d/sonar/Simrad/net/Subscription.h>

namespace gz4d
{
    namespace Simrad
    {
        class GZ4DEXPORT SubscriptionManager: public Consumer<SubscriptionManager, std::shared_ptr<Socket::Packet> >
        {
            Connection::Ptr connection;

            SubscriptionPort::Ptr port;

            boost::mutex subscriptionsMutex;
            std::map<int,Subscription::Ptr> subscriptions;

            std::map<int32_t,std::shared_ptr<Socket::Packet> > partialPackets;

            public:
                typedef std::shared_ptr<SubscriptionManager> Ptr;

                SubscriptionManager(Connection::Ptr &c)
                :connection(c)
                {
                    port = SubscriptionPort::Ptr(new SubscriptionPort(connection->GetLocalAddress().GetAddress()));
                    port->AddOutQueue(GetInQueue());
                }
                
                ~SubscriptionManager()
                {
                    for(std::map<int,Subscription::Ptr>::iterator p = subscriptions.begin(); p != subscriptions.end(); ++p)
                    {
                        Request req (connection,"RemoteDataServer","Unsubscribe");
                        req.AddArgument("subscriptionID",boost::lexical_cast<std::string>(p->first));
                        req.GetResponse();
                    }
                }
                

                void Subscribe(Subscription::Ptr &s)
                {
                    Request req(connection,"RemoteDataServer","Subscribe");
                    req.AddArgument("dataRequest",s->SubscribeString());
                    req.AddArgument("requestedPort",boost::lexical_cast<std::string>(port->GetPort()));
                    
                    boost::mutex::scoped_lock lock(subscriptionsMutex);
                    
                    Response r(req.GetResponse());
                    if(r.GetErrorCode())
                        throw(Exception(r.GetErrorMessage()));
                    int id = boost::lexical_cast<int>(r.GetArgument("subscriptionID"));
                    s->SetID(id);
                    
                    subscriptions[id] = s;
                    
                }
                
                void Update(Subscription::Ptr &s)
                {
                    Request req(connection,"RemoteDataServer","ChangeSubscription");
                    req.AddArgument("subscriptionID",boost::lexical_cast<std::string>(s->GetID()));
                    req.AddArgument("dataRequest",s->SubscribeString());
                    
                    boost::mutex::scoped_lock lock(subscriptionsMutex);
                    Response r(req.GetResponse());
                    if(r.GetErrorCode())
                        throw(Exception(r.GetErrorMessage()));
                }
                
                void Unsubscribe(Subscription::Ptr &sub)
                {
                    Request req(connection,"RemoteDataServer","Unsubscribe");
                    req.AddArgument("subscriptionID",boost::lexical_cast<std::string>(sub->GetID()));
                    Response r(req.GetResponse());
                    if(r.GetErrorCode())
                        throw(Exception(r.GetErrorMessage()));
                }
                

                void Consume(std::shared_ptr<Socket::Packet> packet)
                {
                    //std::cerr << "gz4d::Simrad::SubscriptionManager::Consume packet of length " << packet->size() << std::endl;
                    Packet::ProcessedData *data = reinterpret_cast<Packet::ProcessedData *>(&packet->front());
                    if(*data)
                    {
                        std::shared_ptr<std::vector<unsigned char> > dataPtr;
                        if(data->TotalMsg > 1)
                        {
                            partialPackets[data->SeqNo] = packet;
                            bool complete = true;
                            int32_t first = data->SeqNo-data->CurrentMsg+1;
                            for(int32_t i = first; i < first+data->TotalMsg && complete; ++i)
                                complete = partialPackets.count(i);
                            if(complete)
                            {
                                dataPtr = std::shared_ptr<std::vector<unsigned char> > (new std::vector<unsigned char>);
                                for(int32_t i = first; i < first+data->TotalMsg && complete; ++i)
                                {
                                    data = reinterpret_cast<Packet::ProcessedData *>(&partialPackets[i]->front());
                                    dataPtr->insert(dataPtr->end(),&data->data,&(&data->data)[data->NoOfBytes]);
                                }
                            }
                        }
                        else
                        {
                            dataPtr = std::shared_ptr<std::vector<unsigned char> > (new std::vector<unsigned char>(&data->data,&(&data->data)[data->NoOfBytes]));
                        }
                        if(dataPtr)
                        {
                            boost::mutex::scoped_lock lock(subscriptionsMutex);
                            Subscription::Ptr s(subscriptions[data->SubscriptionID]);
                            if (s)
                                s->AddData(dataPtr);
                        }
                    }
                    else
                    {
                        std::cerr << "Not the subscription packet I was expecting... " << std::endl;
                    }
                }
                
        };


    }
}

#endif

