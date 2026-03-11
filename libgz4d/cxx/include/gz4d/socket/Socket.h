// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SOCKET_H
#define GZ4D_SOCKET_H

#include <vector>
#include <string>
#include <cstring>
#include <gz4d/gz4d.h>
#include <gz4d/Exception.h>
#include <gz4d/data/Binary.h>
#include <memory>

#ifdef BOOST_MSVC
    #define WINVER 0x0501
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifndef EWOULDBLOCK
        #define EWOULDBLOCK WSAEWOULDBLOCK
    #endif
    #ifndef EINTR
        #define EINTR WSAEINTR
    #endif
    //#define socklen_t int
    typedef u_short sa_family_t;
    #define GZ4D_INVALID_SOCKET INVALID_SOCKET
    #define GZ4D_SOCKET_ERROR SOCKET_ERROR
#else
    #include <errno.h>
    #include <sys/fcntl.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #define GZ4D_INVALID_SOCKET -1
    #define GZ4D_SOCKET_ERROR -1
#endif




#ifdef BOOST_MSVC
#pragma comment(lib, "ws2_32.lib")
#endif


namespace gz4d
{
    /// Data communication socket.
    /// \ingroup utilities
    class Socket
    {
        public:

            #ifdef WIN32
                typedef SOCKET Handle;
            #else
                typedef int Handle;
            #endif

            typedef std::vector<char> Packet;

            class Exception: public gz4d::Exception
            {
                public:
                    Exception(std::string msg):gz4d::Exception(msg){}
            };

            class WouldBlockException: public Exception
            {
                public:
                    WouldBlockException(std::string msg="Operation would have blocked"):Exception(msg){}
            };

            class ConnectionClosedException: public Exception
            {
                public:
                    ConnectionClosedException(std::string msg="Connection closed"):Exception(msg){}
            };

            class Interrupted: public Exception
            {
                public:
                    Interrupted(std::string msg="Operation interrupted"):Exception(msg){}
            };

            enum Option {
                Broadcast = SO_BROADCAST,
                ReuseAddress = SO_REUSEADDR,
                SendBuffer = SO_SNDBUF,
                RecvBuffer = SO_RCVBUF,
                SendTimeout = SO_SNDTIMEO,
                RecvTimeout = SO_RCVTIMEO
            };

            /// Base socket address class.
            template <typename T, sa_family_t TF> class Address
            {
                public:
                    bool operator==(Address const &other) const {return memcmp(&_address, &other._address, sizeof(T)) == 0;}
                    bool operator!=(Address const &other) const {return memcmp(&_address, &other._address, sizeof(T)) != 0;}

                protected:
                    T _address;

                    Address()
                    {
                        reinterpret_cast<sockaddr_storage*>(&_address)->ss_family = TF;
                    }

                    Address(Address const &a):_address(a._address){}

                    friend class Socket;

                    socklen_t size() const {return sizeof(T);}
                    operator const sockaddr*() const {return reinterpret_cast<const sockaddr*>(&_address);}
                    operator sockaddr*() {return reinterpret_cast<sockaddr*>(&_address);}
            };

            Socket():_socket(GZ4D_INVALID_SOCKET)
            {
            }

            Socket(Socket const &s):_socket(GZ4D_INVALID_SOCKET)
            {
                #ifdef WIN32
                    win_initialize();

                    WSAPROTOCOL_INFO pi;
                    WSADuplicateSocket(s._socket,GetCurrentProcessId(),&pi);
                    _socket = WSASocket(pi.iAddressFamily,pi.iSocketType,pi.iProtocol,&pi,0,0);
                #else
                    _socket = dup(s._socket);
                #endif
            }
            

            explicit Socket(Handle s):_socket(s)
            {
                #ifdef WIN32
                win_initialize();
                #endif
            }

            ~Socket()
            {
                try
                {
                    Close();
                }
                catch(...)
                {}
            }


            operator Handle() const
            {
                return _socket;
            }

            inline bool IsValid() const
            {
                return _socket != GZ4D_INVALID_SOCKET;
            }

            inline operator bool() const
            {
                return IsValid();
            }


            template <typename T> void Bind(T const &address) const
            {
                HandleError(bind(_socket, address, address.size()));
            }


            template <typename T> void Connect(T const &address) const
            {
                HandleError(connect(_socket, address, address.size()));
            }

            void Listen(int backlog=1, bool reuseAddr=true)
            {
                int t = reuseAddr;
                HandleError(setsockopt(_socket,SOL_SOCKET,SO_REUSEADDR,(char const *)&t,sizeof(t)));
                HandleError(listen(_socket,backlog));
            }

            template <typename T> Socket Accept(T &address)
            {
                socklen_t l = address.size();
                int ret = accept(_socket,address,&l);
                HandleError(ret);
                return Socket(ret);
            }

            void Close()
            {
                #ifdef WIN32
                    HandleError(closesocket(_socket));
                #else
                    HandleError(close(_socket));
                #endif
                _socket = GZ4D_INVALID_SOCKET;
            }

            template <typename T> void Send(T const &data)
            {
                Send(&data[0],data.size());
            }

            void Send(char const *data, int len)
            {
                int numSent = 0;
                int ret;
                do
                {
                    ret = send(_socket,&data[numSent],len-numSent,0);
                    if (ret < 0)
                        try
                        {
                            HandleError(ret);
                        }
                        catch(WouldBlockException)
                        {
                            ret = 0;
                        }
                        numSent += ret;
                }
                while(numSent < len);
                HandleError(ret);
            }

            void Send(binary::Packet const * packet)
            {
                Send(reinterpret_cast<char const *>(packet), packet->size);
            }


            template <typename T, typename AT> void SendTo(T const &data, AT const &address)
            {
                SendTo(&data[0],data.size(),address);
            }

            template <typename AT> void SendTo(char const *data, int len, AT const &address)
            {
                int numSent = 0;
                int ret;
                do
                {
                    ret = sendto(_socket,&data[numSent],len-numSent,0,address,address.size());
                    numSent += ret;
                }
                while(ret > 0 && numSent < len);
                HandleError(ret);
            }

            template <typename AT> void SendTo(binary::Packet const * packet, AT const &address)
            {
                SendTo(reinterpret_cast<char const *>(packet), packet->size, address);
            }

            /// Receive some data from the socket.
            /// Up to data.size() bytes are returned on success.
            template <typename T> void Recv(T &data)
            {
                int ret = recv(_socket,(char *)&data[0],data.size(),0);
                if (ret==0)
                    throw(ConnectionClosedException());
                HandleError(ret);
                data.resize(ret);
            }

            /// Similar to Recv()
            template <typename T, typename AT> void RecvFrom(T &data, AT &address)
            {
                socklen_t l = address.size();
                int ret = recvfrom(_socket,(char *)&data[0],data.size(),0,address,&l);
                if (ret==0)
                    throw(ConnectionClosedException());
                HandleError(ret);
                data.resize(ret);
            }

            template <typename T> void GetLocalAddress(T &address) const
            {
                socklen_t addrlen = address.size();
                HandleError(getsockname(_socket,address,&addrlen));
            }

            template <typename T> void GetRemoteAddress(T &address) const
            {
                socklen_t addrlen = address.size();
                HandleError(getpeername(_socket,address,&addrlen));
            }

            /// Sets socket to non-blocking mode.
            /// When in non-blocking mode, an error is flagged when a call that would block is made.
            /// In such a case, the WouldHaveBlocked() method will return true to distinguise this 
            /// condition from other errors.
            void SetNonBlocking()
            {
                #ifdef WIN32
                    unsigned long nb = 1;
                    HandleError(ioctlsocket(_socket,FIONBIO,&nb));
                #else
                    HandleError(fcntl(_socket, F_SETFL, O_NONBLOCK));
                #endif
            }

            void SetOption(Option option, int value) const
            {
                switch(option)
                {
                    case SendTimeout:
                    case RecvTimeout:
                        timeval tv;
                        tv.tv_sec = value;
                        tv.tv_usec = 0;
                        HandleError(setsockopt(_socket,SOL_SOCKET,option,reinterpret_cast<char const*>(&tv),sizeof(tv)));
                        break;
                    default:
                        HandleError(setsockopt(_socket,SOL_SOCKET,option,reinterpret_cast<char const*>(&value),sizeof(value)));
                }
            }

            int GetOption(Option option) const
            {
                int ret;
                #ifdef WIN32
                    char *pret = (char *)&ret;
                #else
                    int *pret = &ret;
                #endif
                socklen_t len = sizeof(ret);
                HandleError(getsockopt(_socket,SOL_SOCKET,option,pret,&len));
                return ret;
            }

            Socket const &operator=(Socket const &other)
            {
                if(IsValid())
                    Close();

                #ifdef WIN32
                    win_initialize();

                    WSAPROTOCOL_INFO pi;
                    WSADuplicateSocket(other._socket,GetCurrentProcessId(),&pi);
                    _socket = WSASocket(pi.iAddressFamily,pi.iSocketType,pi.iProtocol,&pi,0,0);
                #else
                    _socket = dup(other._socket);
                #endif
                return *this;
            }

        protected:
            Socket(int domain, int type, int protocol)
            {
                #ifdef WIN32
                    win_initialize();
                #endif
                _socket = socket(domain,type,protocol);
                HandleError(_socket,GZ4D_INVALID_SOCKET);
            }

        private:
            Handle _socket;

            inline static void HandleError(int ret, int err=GZ4D_SOCKET_ERROR)
            {
                if (ret == err)
                    #ifdef WIN32
                        switch(WSAGetLastError())
                    #else
                        switch(errno)
                    #endif
                        {
                        case EWOULDBLOCK:
                            throw WouldBlockException(std::string(strerror(errno)));
                        case EINTR:
                            throw Interrupted(std::string(strerror(errno)));
                        default:
                            throw Exception(std::string(strerror(errno)));
                        }
            }

            #ifdef WIN32
            inline static bool win_initialize()
            {
                WORD wVersionRequested = MAKEWORD(2, 0);
                WSADATA wsaData;
                int err;

                err = WSAStartup( wVersionRequested, &wsaData );
                if ( err != 0 ) {
                    return false; //no usable DLL!
                }
                return true;
            }

            inline static void win_shutdown()
            {
                WSACleanup( );
            }
            #endif


    };
};

#endif
