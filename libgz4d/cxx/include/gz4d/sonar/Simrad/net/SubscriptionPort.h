// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_SUBSCRIPTIONPORT_H
#define GZ4D_SIMRAD_SUBSCRIPTIONPORT_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/socket/UDPSocket.h>
#include <boost/thread/thread.hpp>

namespace gz4d
{
    namespace Simrad
    {

        /// Establishes UDP socket and pushes received packets to a queue.
        class SubscriptionPort: public Producer<std::shared_ptr<Socket::Packet> >
        {
            UDPSocket sock;
            int port;

            boost::thread receiverThread;
            void ReceiverThread()
            {
                IPSocket::Address a;
                while (!boost::this_thread::interruption_requested())
                {
                    boost::thread::yield();
                    std::shared_ptr<Socket::Packet> p(new Socket::Packet);
                    p->resize(1500);
                    try
                    {
                        sock.RecvFrom(*p,a);
                        //std::cerr << "subscription packet of size: " << p->size() << " on port " << port << " from " << a.GetPort() << std::endl;
                        Send(p);
                    }
                    catch(Socket::WouldBlockException)
                    {
                    }
                    catch(Socket::Interrupted)
                    {
                    }
                }
            }
            
            

            public:
                typedef std::shared_ptr<SubscriptionPort> Ptr;

                SubscriptionPort(std::string const &address)
                {
                    IPSocket::Address a(address);
                    sock.SetOption(Socket::RecvTimeout,1);
                    sock.Bind(a);
                    sock.GetLocalAddress(a);
                    port = a.GetPort();
                    receiverThread = std::move(boost::thread(boost::bind(&SubscriptionPort::ReceiverThread,this)));
                }
                
                ~SubscriptionPort()
                {
                    receiverThread.interrupt();
                    receiverThread.join();
                }
                
                
                int GetPort() const {return port;}
        };
    }
}

#endif

