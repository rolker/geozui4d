#ifndef GZ4D_SIMRAD_CONNECTION_H
#define GZ4D_SIMRAD_CONNECTION_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Packet.h>
#include <gz4d/thread/ThreadedQueue.h>
#include <gz4d/socket/UDPSocket.h>

namespace gz4d
{
    namespace Simrad
    {
        class Connection
        {
            UDPSocket sock;
            unsigned int nextSeqNo; /// Number that will be used for the next message to the server.
            unsigned int lastServerSeqNo; 

            Packet::DisconnectRequest disconnect;
            std::thread connectionThread;

            IPSocket::Address server;

            std::string clientID; /// Server supplied ID.

            int reqID;

            std::vector<Packet::Request> pendingRequest;
            bool sendRequest;
            std::mutex requestMutex;

            std::string pendingResponse;
            std::mutex responseMutex;
            std::condition_variable responseAvailable;

            public:
                typedef std::shared_ptr<Connection> Ptr;

                Connection(IPSocket::Address const &a, std::string uname, std::string pwd)
                :server(server),sendRequest(false)
                {
                    Packet::ConnectRequest request;
                    request.SetNamePwd(uname,pwd);
                    
                    sock.SetNonBlocking();
                    sock.Connect(server);
                    sock.Send(request,sizeof(request));
                    
                    Socket::Packet responsePacket(sizeof(Packet::Response));
                    Packet::Response *response = (Packet::Response*)&responsePacket.front();
                    
                    gz4d::Time timeout = gz4d::time::Now() +time::Duration(10.0);
                    while(gz4d::time::Now() < timeout)
                    {
                        try
                        {
                            sock.Recv(responsePacket);
                            if(*response)
                                break;
                            responsePacket.resize(sizeof(Packet::Response));
                        }
                        catch (Socket::WouldBlockException)
                        {
                            //std::cerr << "gz4d::Simrad::Connection::Connection waiting...\n";
                            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                        }
                        catch (Socket::Exception e)
                        {
                            std::cerr << "gz4d::Simrad::Connection::Connection Error establishing connection to " << server.GetAddress() << ":" << server.GetPort() << " - " << e.what() << std::endl;
                        }
                    }
                    
                    if(!(bool)(*response))
                        throw(Exception("Timeout waiting for connection response"));
                    
                    ParameterMap result(response->GetResult());
                    
                    if (result.find("ClientID") == result.end())
                        throw(Exception("ClientID not found in connection response"));
                    
                    
                    clientID = result["ClientID"];
                    //std::cerr << "Client ID: " << clientID << std::endl;
                    nextSeqNo = 1;
                    lastServerSeqNo = 0;
                    reqID = 1;
                    disconnect.SetNamePwd(uname,pwd);
                    connectionThread = std::move(std::thread(std::ref(*this)));
                    boost::this_thread::sleep(boost::posix_time::seconds(1));
                }
                
                ~Connection()
                {
                    //connectionThread.interrupt();
                    connectionThread.join();
                }
                

                std::string GetID(){return clientID;}
                
                std::string SendRequest(std::string const &req)
                {
                    std::unique_lock<std::mutex> req_lock(requestMutex);
                    std::unique_lock<std::mutex> res_lock(responseMutex);
                    pendingResponse = "";
                    
                    int parts = (int)ceil(req.size()/1400.0);
                    for(int i = 0; i < parts; ++i)
                    {
                        Packet::Request r;
                        int count = req.size()+1-(i*1400);
                        if(count > 1400)
                            count = 1400;
                        memcpy(r.MsgResponse,req.substr(i*1400).c_str(),count);
                        pendingRequest.push_back(r);
                    }
                    
                    sendRequest = true;
                    while(pendingResponse.empty())
                        responseAvailable.wait(res_lock);
                    return pendingResponse;
                }
                

                int GetRID()
                {
                    return reqID++;
                }
                

                void operator()()
                {
                    gz4d::Time lastSentRequest;
                    
                    while(!boost::this_thread::interruption_requested())
                    {
                        {
                            std::unique_lock<std::mutex> lock(responseMutex);
                            try
                            {
                                Socket::Packet packet(1500);
                                sock.Recv(packet);
                                
                                Packet::AliveReport* server_alive = (Packet::AliveReport*)&packet.front();
                                Packet::Response* response = (Packet::Response*)&packet.front();
                                Packet::Retransmit* rtr = (Packet::Retransmit*)&packet.front();
                                
                                if(*server_alive)
                                {
                                    Packet::AliveReport alive;
                                    alive.SetInfo(clientID,nextSeqNo);
                                    sock.Send(alive,sizeof(alive));
                                }
                                else if(*rtr)
                                {
                                    std::cerr << "retransmit request from server" << std::endl;
                                }
                                else if (*response)
                                {
                                    std::string msgCtrl(response->MsgControl);
                                    lastServerSeqNo = boost::lexical_cast<unsigned int>(msgCtrl.substr(0,msgCtrl.find(",")));
                                    pendingResponse = response->MsgResponse;
                                    responseAvailable.notify_all();
                                    pendingRequest.clear();
                                    lastSentRequest = Time();
                                }
                            }
                            catch(Socket::WouldBlockException)
                            {}
                            if(!pendingRequest.empty())
                            {
                                if (!lastSentRequest.valid() || lastSentRequest+time::Duration(5.0) < gz4d::time::Now())
                                {
                                    for(int i = 0; i < pendingRequest.size(); ++i)
                                    {
                                        std::string msgctrl = boost::lexical_cast<std::string>(nextSeqNo) + "," + boost::lexical_cast<std::string>(i+1)+","+boost::lexical_cast<std::string>(pendingRequest.size());
                                        strcpy(pendingRequest[i].MsgControl,msgctrl.substr(0,21).c_str());
                                        sock.Send(pendingRequest[i],sizeof(pendingRequest[i])-1399+strlen(pendingRequest[i].MsgResponse));
                                        nextSeqNo += 1;
                                    }
                                    lastSentRequest = gz4d::time::Now();
                                }
                            }
                        }
                        boost::thread::yield();
                    }
                    std::cerr << "gz4d::Simrad::Connection disconnecting" << std::endl;
                    sock.Send(disconnect,sizeof(disconnect));
                    
                }
                

                IPSocket::Address GetLocalAddress() const
                {
                    gz4d::IPSocket::Address ret;
                    sock.GetLocalAddress(ret);
                    return ret;
                }
                
        };

    }
}

#endif
