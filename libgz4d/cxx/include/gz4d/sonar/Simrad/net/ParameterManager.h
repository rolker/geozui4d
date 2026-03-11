// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_PARAMETERMANAGER_H
#define GZ4D_SIMRAD_PARAMETERMANAGER_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/Parameter.h>
#include <gz4d/sonar/Simrad/net/SubscriptionPort.h>
#include <gz4d/sonar/Simrad/net/Connection.h>
#include <gz4d/sonar/Simrad/net/Request.h>
#include <gz4d/thread/Consumer.h>

namespace gz4d
{
    namespace Simrad
    {
        /// Manages subscriptions to parameters.
        class ParameterManager: public Consumer<ParameterManager, std::shared_ptr<Socket::Packet> >
        {
            SubscriptionPort::Ptr subscriptionPort;

            Connection::Ptr connection;

            boost::mutex paramsMutex;
            std::map<int,Parameter::Ptr> params;
            std::map<int,std::string> paramNames;

            public:
                typedef std::shared_ptr<ParameterManager> Ptr;

                ParameterManager(Connection::Ptr &c, SubscriptionPort::Ptr &p)
                :connection(c),subscriptionPort(p)
                {
                    subscriptionPort = SubscriptionPort::Ptr(new SubscriptionPort(connection->GetLocalAddress().GetAddress()));
                    subscriptionPort->AddOutQueue(GetInQueue());
                }
                
                ~ParameterManager()
                {
                    for(std::map<int,Parameter::Ptr>::iterator p = params.begin(); p != params.end(); ++p)
                    {
                        Request req (connection,"ParameterServer","CloseSubscription");
                        req.AddArgument("paramName",paramNames[p->first]);
                        req.AddArgument("cookie",boost::lexical_cast<std::string>(p->first));
                        req.GetResponse();
                    }
                }
                

                Parameter::Ptr Subscribe(std::string const &paramName, bool useDefault = true)
                {
                    bool valIsTime = false;
                    if (!boost::find_last(paramName,"/PingTime").empty())
                        valIsTime = true;
                    
                    Parameter::Ptr ret(new Parameter(GetInfo(paramName),useDefault,valIsTime));
                    
                    Request req (connection,"ParameterServer","Subscribe");
                    req.AddArgument("paramName",paramName);
                    req.AddArgument("requestedPort",boost::lexical_cast<std::string>(subscriptionPort->GetPort()));
                    
                    Response r(req.GetResponse());
                    int id = boost::lexical_cast<int>(r.GetArgument("cookie"));
                    
                    boost::mutex::scoped_lock lock(paramsMutex);
                    params[id] = ret;
                    paramNames[id] = paramName;
                    
                    return ret;
                }
                
                
                Parameter::Info GetInfo(std::string const &paramName)
                {
                    Request req(connection,"ParameterServer","GetParamInfo");
                    req.AddArgument("paramName",paramName);
                    Response r(req.GetResponse());
                    switch (r.GetErrorCode())
                    {
                        case 0:
                            // no problem!
                            break;
                        case -2147467259:
                            // ER60 parameter not found
                            throw(InvalidParameterException(r.GetErrorMessage()));
                        default:
                            std::cerr << static_cast<std::string>(r) << std::endl;
                            throw(Exception(r.GetErrorMessage()));
                    }
                    
                    Parameter::Info info;
                    
                    std::string stype = r.GetArgument("paramInfo/attributes/Type");
                    if (!stype.size()) // handle silly Simrad inconsistencies between ME70 and ER60 software
                    {
                        // ER60
                        stype = r.GetArgument("paramInfo/attributes/type");
                        info.name = r.GetArgument("paramInfo/attributes/name");
                        info.description = r.GetArgument("paramInfo/attributes/description");
                        info.defaultValue = r.GetArgument("paramInfo/attributes/default");
                    }
                    else
                    {
                        info.name = r.GetArgument("paramInfo/attributes/Name");
                        info.description = r.GetArgument("paramInfo/attributes/Description");
                        info.defaultValue = r.GetArgument("paramInfo/attributes/Default");
                    }
                    
                    info.type = boost::lexical_cast<int>(stype);
                    
                    return info;
                }
                

                std::string Get(std::string const &paramName)
                {
                    
                    Request req(connection,"ParameterServer","GetParameter");
                    req.AddArgument("paramName",paramName);
                    req.AddArgument("time","0");
                    Response r(req.GetResponse());
                    switch (r.GetErrorCode())
                    {
                        case 0:
                            // no problem!
                            break;
                        case -2147467259:
                            // ER60 parameter not found
                            throw(InvalidParameterException(r.GetErrorMessage()));
                        default:
                            std::cerr << static_cast<std::string>(r) << std::endl;
                            throw(Exception(r.GetErrorMessage()));
                    }
                    
                    if(r.GetErrorCode())
                        throw(Exception(r.GetErrorMessage()));
                    
                    return r.GetArgument("paramValue/value");
                }
                

                void Consume (std::shared_ptr<Socket::Packet> packet)
                {
                    
                    #pragma pack(1)
                    
                    struct Record
                    {
                        unsigned short recID;
                        unsigned short recLen;
                    };
                    
                    struct Header
                    {
                        unsigned short currentMsg;
                        unsigned short totalMsg;
                        int32_t seqNo;
                        int32_t msgID;
                        Record firstRecord;
                    };
                    
                    struct Value
                    {
                        int32_t len;
                        char value;
                        int Size(){return len+sizeof(len);}
                    };
                    
                    struct ParameterDefinition
                    {
                        int32_t cookie;
                        uint64_t timeStamp;
                        Value value;
                        int32_t unused;
                        int Size(){return sizeof(ParameterDefinition) - sizeof(value) + value.Size();}
                    };
                    
                    struct ParameterRecord: public Record
                    {
                        int32_t paramCount;
                        ParameterDefinition parameters;
                    };
                    
                    #pragma pack()
                    
                    //std::cerr << "ParameterManager::Consume packet of size " << packet->size();
                    int cursor = sizeof(Header);
                    
                    Header *h = (Header*)(&packet->front());
                    //std::cerr << "\tMsg: " << h->currentMsg << " of " << h->totalMsg << "\tSeq No: " << h->seqNo << "\tid:" << h->msgID << std::endl;
                    if(h->currentMsg > 1)
                        throw(Exception("Fragmented update packet not yet supported!"));
                    
                    if(h->msgID == 6) //PM_VALUE_ATTRIBUTE_UPDATE = 6
                    {
                        Record *rec = &h->firstRecord;
                        bool done = false;
                        while(!done)
                        {
                            switch (rec->recID)
                            {
                                case 11: // value update
                                {
                                    ParameterRecord *pr = (ParameterRecord *)rec;
                                    //std::cerr << "\tValue update with " << pr->paramCount << " records." << std::endl;
                                    ParameterDefinition *param = &pr->parameters;
                                    for(int i = 0; i<pr->paramCount; ++i)
                                    {
                                        //std::cerr << "ID:\t" << param->cookie << " time: " << gz4d::Time::FromNT(param->timeStamp).String() << " (" << param->timeStamp << ")" << std::endl;
                                        //std::cerr << "\tvalue len: " << param->value.len << std::endl;
                                        
                                        //if(param->timeStamp)
                                        {
                                            ParameterUpdate update;
                                            update.id = param->cookie;
                                            update.time = gz4d::time::FromNT(param->timeStamp);
                                            char *v = &param->value.value;
                                            update.value = std::vector<char>(v,&v[param->value.len]);
                                            Parameter::Ptr p;
                                            while(!p)
                                            {
                                                {
                                                    boost::mutex::scoped_lock lock(paramsMutex);
                                                    p = params[update.id];
                                                }
                                                if (!p)
                                                    boost::this_thread::yield();
                                            }
                                            p->Update(update);
                                        }
                                        param =  (ParameterDefinition*)&(((char *)param)[param->Size()]);
                                    }
                                    break;
                                }
                                case 12: // attribute update
                                    //std::cerr << "\tAttribue update" << std::endl;
                                    break;
                                case 0xdead: // end of records
                                    done = true;
                                    break;
                                default:
                                    break;
                            }
                            rec = reinterpret_cast<Record*>(&(reinterpret_cast<char *>(rec)[rec->recLen]));
                        }
                    }
    
                }
                

        };


    }
}



#endif
