#ifndef GZ4D_SOCKET_RECEIVER_H
#define GZ4D_SOCKET_RECEIVER_H
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "../thread/Producer.h"
#include "../URI.h"
#include <fstream>

namespace gz4d
{
    class SocketReceiver: public Producer<std::shared_ptr<Socket::Packet> >
    {
        boost::thread socketThread;

        public:
            typedef std::shared_ptr<SocketReceiver> Ptr;

            SocketReceiver(Socket const &s)
            {
                socketThread = std::move(boost::thread(boost::ref(*this),s));
            }

            SocketReceiver(URI const &uri)
            {
                socketThread = std::move(boost::thread(boost::ref(*this),uri));
            }
            
            ~SocketReceiver()
            {
                socketThread.interrupt();
                socketThread.join();
            }

            void operator()(URI const &uri)
            {
                if(uri.scheme == "tcp")
                {
                    gz4d::IPSocket::Address a(uri);
                    TCPSocket socket = gz4d::TCPSocket();
                    socket.Connect(a);
                    operator()(socket);
                }
                else if(uri.scheme == "udp")
                {
                    gz4d::IPSocket::Address a;
                    a.SetPort(uri.getPort());
                    UDPSocket socket = gz4d::UDPSocket();
                    socket.Bind(a);
                    operator()(socket);
                }
                else if(uri.scheme == "file")
                {
                    std::ifstream in(uri.path.c_str());
                    while (!boost::this_thread::interruption_requested() && in.good())
                    {
                        boost::thread::yield();
                        std::shared_ptr<Socket::Packet> packet(new Socket::Packet(65000));
                        in.read(&(packet->front()),65000);
                        int gc = in.gcount();
                        if(gc)
                        {
                            packet->resize(gc);
                            Send(packet);
                        }
                    }                    
                    
                }
            }
            
            void operator()(Socket const &s)
            {
                Socket socket(s);
                socket.SetOption(Socket::RecvTimeout,1);

                while (!boost::this_thread::interruption_requested())
                {
                    boost::thread::yield();
                    std::shared_ptr<Socket::Packet> packet(new Socket::Packet(65000));
                    try
                    {
                        socket.Recv(*packet);
                        Send(packet);
                    }
                    catch(Socket::WouldBlockException)
                    {
                    }
                    catch(Socket::Interrupted)
                    {
                    }
                    catch (boost::thread_interrupted)
                    {
                        break;
                    }
                }
            }
            
    };
}

#endif
