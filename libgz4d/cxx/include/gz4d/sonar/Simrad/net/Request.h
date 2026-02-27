#ifndef GZ4D_SIMRAD_REQUEST_H
#define GZ4D_SIMRAD_REQUEST_H

#include <gz4d/sonar/Simrad/net/Connection.h>
#include <gz4d/sonar/Simrad/net/Response.h>
#include <boost/property_tree/xml_parser.hpp>

namespace gz4d
{
    namespace Simrad
    {
        class Request
        {
            protected:
                boost::property_tree::ptree request;
                std::string method;
                Connection::Ptr connection;
                unsigned int rid;
            public:
                Request(Connection::Ptr &c, std::string const &target, std::string const &method)
                :method(method),connection(c)
                {
                    request.put("request.clientInfo.cid",c->GetRID());
                    rid = c->GetRID();

                    request.put("request.type","invokeMethod");
                    request.put("request.targetComponent",target);
                    
                }
                
                void AddArgument(std::string const &arg, std::string const &value)
                {
                    request.put("request."+method+"."+arg,value);
                }
                
                Response GetResponse()
                {
                    while(true)
                    {
                        try
                        {
                            std::stringstream r;
                            write_xml(r,request);
                            return Response(method,connection->SendRequest(r.str()),rid);
                        }
                        catch (Exception e)
                        {
                            std::cerr << "gz4d::Simrad::Request::GetResponse\t" << e.what() << std::endl;
                        }
                    }
                }
                
        };

    }
}

#endif
