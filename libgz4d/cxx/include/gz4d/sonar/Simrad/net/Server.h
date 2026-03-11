// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_SERVER_H
#define GZ4D_SIMRAD_SERVER_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Packet.h>
#include <gz4d/socket/IPSocket.h>
#include <gz4d/util/Swap.h>

namespace gz4d
{
    namespace Simrad
    {
        /// Represents a Simrad server on the network.
        /// Contains the necessary info to create a Client connection to the represented server.
        class Server
        {
            IPSocket::Address address;
            bool swap; /// True if byte swapping is needed.

            struct
            {
                std::string appType; ///< Application type as supplied by info request
                std::string appName; ///< Application type as supplied by info request
                std::string appDesc; ///< Application type as supplied by info request
                int appID; ///< Application ID as supplied by info request
                int mode; ///< Mode indicating if application is running against remote or local data source, supplied by info request
                std::string hostName; ///< Host name as supplied by info request
            } info;

            public:
                typedef std::vector<Server> List;

                Server():address(IPSocket::NoAddress()),swap(false){}
                Server(Server const &s):address(s.address),swap(s.swap),info(s.info){}
                Server(IPSocket::Address const &a, Packet::ServerInfo2 const &serverInfo)
                {
                    if(std::string(serverInfo.Header) == "SI2")
                    {
                        // use the port number to attempt to detect byte order. A valid IP port should fit in a 2 byte unsigned int (aka unsigned short)
                        // so if the supplied value seems larger, assume byte swapping is necessary.
                        swap = false;
                        if (serverInfo.CommandPort > 0xffff)
                            swap = true;
                        
                        // check if we got the swapping right
                            if (Swap(serverInfo.CommandPort,swap) > 0xffff)
                                throw (Exception("Problem determining if byte swapping is necessary"));
                            
                            address = IPSocket::Address(a.GetAddress(),Swap(serverInfo.CommandPort,swap));
                            
                            info.appType = serverInfo.ApplicationType;
                            info.appName = serverInfo.ApplicationName;
                            info.appDesc = serverInfo.ApplicationDescription;
                            info.appID = Swap(serverInfo.ApplicationID,swap);
                            info.mode = Swap(serverInfo.Mode,swap);
                            info.hostName = serverInfo.HostName;
                    }
                    else
                        throw(Exception("Invalid packet type. Expected ServerInfo2 structure."));
                }
                

                IPSocket::Address const &GetAddress() const {return address;}
                int GetID() const {return info.appID;}
                std::string GetType() const {return info.appType;}
                std::string String() const
                {
                    std::stringstream s;
                    s << address.GetAddress() << " ";
                    s << "ID:" << info.appID << " ";
                    s << "Type:" << info.appType << " ";
                    s << "Name:" << info.appName << " ";
                    s << "Mode:" << info.mode << " ";
                    s << "Desc:" << info.appDesc;
                    return s.str();
                }
                

                bool operator==(Server const &s) const {return address == s.address;}
                operator bool() const {return address != IPSocket::NoAddress();}

                
        };

    }
}

#endif
