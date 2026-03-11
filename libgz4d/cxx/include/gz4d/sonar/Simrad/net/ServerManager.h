// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_SERVERMANAGER_H
#define GZ4D_SIMRAD_SERVERMANAGER_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Server.h>
#include <gz4d/socket/UDPSocket.h>
#include <gz4d/time.h>
#include <boost/property_tree/xml_parser.hpp>
#include <list>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace gz4d
{
    namespace Simrad
    {
        /// Manages servers available on the network.
        /// Sends a request for servers, and keeps a list of the responses.
        class ServerManager
        {
            uint16_t port;
            std::string localAddress;
            std::string remoteAddress;

            Server::List serverList;
            boost::mutex listMutex;

            boost::thread searchThread;

            void SearchThread()
            {
                UDPSocket s;
                IPSocket::Address a(localAddress);
                s.SetOption(Socket::RecvTimeout,1);
                s.SetOption(Socket::Broadcast,true);
                s.Bind(a);
                
                IPSocket::Address b(remoteAddress,port);
                
                gz4d::Time lastBroadcast(0.0);
                Socket::Packet response(1500);
                IPSocket::Address r;
                
                Packet::RequestServerInfo infoReq;
                
                while (!boost::this_thread::interruption_requested())
                {
                    gz4d::Time now = gz4d::time::Now();
                    if (now > (lastBroadcast +time::Duration(2.5)))
                    {
                        s.SendTo(infoReq, sizeof(infoReq), b); // broadcast request for info packet
                        lastBroadcast = now;
                    }
                    
                    try
                    {
                        response.resize(1500);
                        s.RecvFrom(response,r);
                        Packet::ServerInfo2 *si = (Packet::ServerInfo2*)&response.front();
                        if (*si)
                        {
                            Server s(r,*si);
                            boost::mutex::scoped_lock lock(listMutex);
                            bool exists = false;
                            for(Server::List::iterator i = serverList.begin(); i != serverList.end(); i++)
                            {
                                if(*i == s)
                                {
                                    exists = true;
                                    break;
                                }
                            }
                            if(!exists)
                            {
                                serverList.push_back(s);
                            }
                        }
                    }
                    catch(Socket::WouldBlockException)
                    {
                    }
                    catch(Socket::Interrupted)
                    {
                    }
                }
            }
            
            void LaunchThread()
            {
                searchThread = boost::thread(boost::bind(&ServerManager::SearchThread,this));
            }
            
            public:
                ServerManager(uint16_t p=37655, std::string lAddress="ANY", std::string rAddress="BROADCAST")
                :port(p),localAddress(lAddress),remoteAddress(rAddress)
                {
                    LaunchThread();
                }
                
                ServerManager(boost::property_tree::ptree const &xml)
                :port(37655),localAddress("ANY"),remoteAddress("BROADCAST")
                {
                    port = xml.get("<xmlattr>.port",port);
                    localAddress = xml.get("<xmlattr>.local_address",localAddress);
                    remoteAddress = xml.get("<xmlattr>.remote_address",remoteAddress);
                    LaunchThread();
                }
                
                
                ServerManager(ServerManager const &other)
                :port(other.port),localAddress(other.localAddress),remoteAddress(other.remoteAddress)
                {
                    LaunchThread();
                }
                

                ~ServerManager()
                {
                    searchThread.interrupt();
                    searchThread.join();
                }
                

                Server::List GetList()
                {
                    Server::List ret;
                    boost::mutex::scoped_lock lock(listMutex);
                    ret = serverList;
                    return ret;
                }
                
                Server Get(std::string address)
                {
                    Server ret;
                    boost::mutex::scoped_lock lock(listMutex);
                    for(Server::List::iterator i = serverList.begin(); i != serverList.end(); i++)
                        if (i->GetAddress().GetAddress() == address)
                        {
                            ret = *i;
                            break;
                        }
                        return ret;
                }
                
                Server Get(int id)
                {
                    Server ret;
                    boost::mutex::scoped_lock lock(listMutex);
                    for(Server::List::iterator i = serverList.begin(); i != serverList.end(); i++)
                        if (i->GetID() == id)
                        {
                            ret = *i;
                            break;
                        }
                        return ret;
                }
                
                Server Get(boost::property_tree::ptree const &xml)
                {
                    Server ret;
                    
                    int id=0;
                    bool hasID = false;
                    if(xml.count("Client.<xmlattr>.id"))
                    {
                        hasID = true;
                        id = xml.get("Client.<xmlattr>.id",id);
                    }
                        
                    std::string address;
                    bool hasAddress = false;
                    if(xml.count("Client.<xmlattr>.address"))
                    {
                        hasAddress = true;
                        address = xml.get("Client.<xmlattr>.address",address);
                    }
                    
                        
                    boost::mutex::scoped_lock lock(listMutex);
                    for(Server::List::iterator i = serverList.begin(); i != serverList.end() && !ret; i++)
                    {
                        bool found = true;
                        
                        if (hasID && i->GetID() != id)
                            found = false;
                        
                        if (hasAddress && i->GetAddress().GetAddress() != address)
                            found = false;
                        
                        if(found)
                            ret = *i;
                    }
                    return ret;
                }
                
        };

    }
}

#endif
