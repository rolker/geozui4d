#ifndef GZ4D_SIMRAD_PACKET_H
#define GZ4D_SIMRAD_PACKET_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad.h>
#include <cstring>
#include <boost/lexical_cast.hpp>

namespace gz4d
{
    namespace Simrad
    {
        /// UDP data packets.
        namespace Packet
        {
            struct Base
            {
                char Header[4];
                Base();
                operator char const*() const {return (char const *)this;}
                protected:
                    Base(std::string type){memset(Header,0,4); strcpy(Header,type.substr(0,3).c_str());}
                    bool IsHeader(std::string h) const {return h == Header;}
            };

            struct RequestServerInfo: public Base
            {
                RequestServerInfo():Base("RSI"){}
                operator bool() const {return IsHeader("RSI");}
            };

            /// ServerInfo2 from Simrad EM70 interface specification
            struct ServerInfo2: public Base //"SI2\0"
            {
                char ApplicationType[64];
                char ApplicationName[64]; // Name of the current application
                char ApplicationDescription[128]; // Description of the current application
                int32_t ApplicationID; // ID of the current application
                int32_t CommandPort; // Port number to send commands to
                int32_t Mode; // Whether the application is running against the local data source or a remote data source
                char HostName[64]; // IP address of the computer the application is running on

                operator bool() const {return IsHeader("SI2");}

            };


            struct ClientInfo: public Base
            {
                char Info[1024];
                protected:
                    ClientInfo(std::string type):Base(type){memset(Info,0,1024);}
                    void SetInfo(std::string const &info)
                    {
                        if(info.size() > 1023)
                            throw(Exception("Data too long to fit in packet"));
                        memset(Info,0,1024);
                        strcpy(Info,info.c_str());
                    }
                    
            };

            /// Connection management base
            struct Connection: public ClientInfo
            {
                void SetNamePwd(std::string username = "Simrad", std::string password = std::string())
                {
                    ParameterMap parms;
                    parms["Name"] = username;
                    parms["Password"] = password;
                    parms.SetJoiner(";");
                    SetInfo(parms);
                }
                
                protected:
                    Connection(std::string type):ClientInfo(type){}
            };

            struct ConnectRequest: public Connection
            {
                ConnectRequest():Connection("CON"){}
            };

            struct DisconnectRequest: public Connection
            {
                DisconnectRequest():Connection("DIS"){}
            };

            /// Alive message sent from both the client and server
            struct AliveReport: public ClientInfo
            {
                AliveReport():ClientInfo("ALI"){}
                operator bool() const {return IsHeader("ALI");}
                void SetInfo(std::string clientID, int seqNo)
                {
                    ParameterMap parms;
                    parms["ClientID"] = clientID;
                    parms["SeqNo"] = boost::lexical_cast<std::string>(seqNo);
                    ClientInfo::SetInfo(parms);
                }
                
            };

            /// Response from Simrad EM70 interface specification
            struct Response: public Base
            {
                char Request[4];
                char MsgControl[22];
                char MsgResponse[1400];
                operator bool() const {return IsHeader("RES");}
                ParameterMap GetResult() const
                {
                    ParameterMap parms(MsgResponse);
                    if (parms.find("ResultCode") == parms.end())
                        throw(Exception("Invalid response packet"));
                    
                    if (parms["ResultCode"]!= "S_OK")
                    {
                        std::string reason("Unknown connection error");
                        if(parms.find("ResultInfo") != parms.end())
                            reason = parms["ResultInfo"];
                        
                        if (parms["ResultCode"] == "E_ACCESSDENIED")
                            throw(AccessDeniedException(reason));
                        throw(Exception(reason));
                    }
                    
                    if(parms.find("Parameters") == parms.end())
                        throw(Exception("Unexpected parameter in OK response"));
                    boost::trim_if(parms["Parameters"],boost::is_any_of("{}"));
                    return ParameterMap(parms["Parameters"]);
                }
                
            };

            /// Retransmit request from Simrad EM70
            struct Retransmit: public Base
            {
                char MsgControl[22];
                operator bool() const {return IsHeader("RTR");}
            };

            
            /// Request to server
            struct Request: public Base
            {
                char MsgControl[22];
                char MsgResponse[1400];
                Request():Base("REQ"){memset(MsgControl,0,1422);}
            };

            /// Subscription data
            struct ProcessedData: public Base
            {
                int32_t SeqNo;
                int32_t SubscriptionID;
                unsigned short CurrentMsg;
                unsigned short TotalMsg;
                unsigned short NoOfBytes;
                unsigned char data;
                operator bool() const {return IsHeader("PRD");}
            };
        }

    }
}

#endif
