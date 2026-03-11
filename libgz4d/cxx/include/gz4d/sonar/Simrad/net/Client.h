// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_CLIENT_H
#define GZ4D_SIMRAD_CLIENT_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Server.h>
#include <gz4d/sonar/Simrad/net/Connection.h>
#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/Platform.h>
#include <gz4d/sonar/Simrad/net/Transducer.h>
#include <gz4d/sonar/Simrad/net/BeamConfigurationManager.h>

namespace gz4d
{
    namespace Simrad
    {
        /// Represents a client session with a Simrad server.
        /// Establishes a connection and initiates managers for paramters and subscriptions.
        class Client
        {
            Connection::Ptr connection;

            Server server;

            Platform::Ptr platform;

            Transducer::Ptr transducer;
            BeamConfigurationManager::Ptr beamConfigurationManager;

            SubscriptionPort::Ptr subscriptionPort;
            ParameterManager::Ptr parameterManager;

            SubscriptionManager::Ptr subscriptionManager;

            
            public:
                typedef std::shared_ptr<Client> Ptr;

                Client(Server const &s)
                :server(s)
                {
                }
                
                static Ptr Create(Server const &s)
                {
                    Ptr ret(new  Client(s));
                    return ret;
                }
                
                

                void Connect(std::string uname = "Simrad", std::string pwd = std::string())
                {
                    connection = Connection::Ptr(new Connection(server.GetAddress(),uname,pwd));
                    
                    subscriptionPort = SubscriptionPort::Ptr(new SubscriptionPort(connection->GetLocalAddress().GetAddress()));
                    parameterManager = ParameterManager::Ptr(new ParameterManager(connection,subscriptionPort));
                    
                    subscriptionPort->AddOutQueue(parameterManager->GetInQueue());
                    
                    platform = Platform::Ptr(new Platform(parameterManager));
                    
                    beamConfigurationManager = BeamConfigurationManager::Ptr(new BeamConfigurationManager(parameterManager));
                    
                    subscriptionManager = SubscriptionManager::Ptr(new SubscriptionManager(connection));
                    
                    transducer = Transducer::Ptr(new Transducer(parameterManager,subscriptionManager));
                }
                

                Platform::Ptr &GetPlatform()
                {
                    return platform;
                }
                
                Transducer::Ptr &GetTransducer()
                {
                    return transducer;
                }
                
                BeamConfigurationManager::Ptr &GetBeamConfigurationManager()
                {
                    return beamConfigurationManager;
                }
                
        };
    }
}

#endif
