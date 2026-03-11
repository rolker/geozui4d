// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_IPSOCKET_H
#define GZ4D_IPSOCKET_H

#include "Socket.h"
#include "../URI.h"

#ifdef WIN32
    #include <ws2tcpip.h>
    typedef uint16_t in_port_t;
#else
    #include <arpa/inet.h>
    #include <net/if.h>
    #include <netdb.h>
#endif


namespace gz4d
{
    inline bool operator== (in_addr const & a, in_addr const &b)
    {
        return a.s_addr == b.s_addr;
    }

    inline bool operator== (sockaddr_in const  &a,sockaddr_in const &b)
    {
        return a.sin_addr == b.sin_addr && a.sin_port == b.sin_port;
    }


    class IPSocket: public Socket
    {
        public:
            class Address: public Socket::Address<sockaddr_in, AF_INET>
            {
                public:
                    Address(){SetAddress("ANY"); SetPort(0);}
                    Address(std::string const &address, in_port_t port=0){SetAddress(address); SetPort(port);}
                    Address(URI const &uri){SetAddress(uri.host); SetPort(uri.getPort());}

                    void SetPort(in_port_t port){ _address.sin_port = htons(port);}
                    void SetAddress(std::string const &addr )
                    {
                        if (addr == "ANY")
                            _address.sin_addr.s_addr = htonl(INADDR_ANY);
                        else if (addr == "BROADCAST")
                            _address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
                        else if (addr.empty() || addr == "NONE")
                            _address.sin_addr.s_addr = htonl(INADDR_NONE);
                        else if (addr == "LOOPBACK")
                            _address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                        else
                        {
                            addrinfo *ai;
                            addrinfo hints = {0};
                            hints.ai_family = AF_INET;
                            int ret = getaddrinfo(addr.c_str(),NULL,&hints,&ai);
                            if(ret)
                                throw Exception(std::string("Address resolution error: ")+addr+" "+gai_strerror(ret));
                            _address.sin_addr.s_addr = ((sockaddr_in*)(ai->ai_addr))->sin_addr.s_addr;
                            freeaddrinfo(ai);
                        }
                    }

                    std::string GetAddress() const
                    {
                        return inet_ntoa(_address.sin_addr);
                    }

                    in_port_t GetPort() const
                    {
                        return ntohs(_address.sin_port);
                    }
            };

            inline static Address NoAddress(){return Address("NONE");}

            inline static std::vector<std::string> Interfaces()
            {
                std::vector<std::string> ret;

                #ifndef WIN32
                //UDPSocket s;
                IPSocket s;
                ifconf ifc;
                std::vector<char> buf(1024);
                ifc.ifc_len = buf.size();
                ifc.ifc_buf = &buf.front();
                if(ioctl(s,SIOCGIFCONF, &ifc) < 0)
                    throw Exception(std::string(strerror(errno)));
                for(ifreq *ifr = ifc.ifc_req; ifr->ifr_name[0]; ++ifr)
                {
                    std::string as = inet_ntoa(((sockaddr_in *)&ifr->ifr_addr)->sin_addr);
                    ret.push_back(as);
                }
                #endif

                return ret;
            }


            IPSocket(){}
        protected:
            IPSocket(int type):Socket(AF_INET,type,0){}
            
    };

    
}

#endif
