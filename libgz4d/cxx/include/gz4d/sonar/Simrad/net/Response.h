// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_RESPONSE_H
#define GZ4D_SIMRAD_RESPONSE_H

#include <gz4d/sonar/Simrad.h>
#include <boost/property_tree/xml_parser.hpp>
#include <vector>
#include <memory>

namespace gz4d
{
    namespace Simrad
    {
        class Response
        {
            protected:
                boost::property_tree::ptree response;
                std::string method;
                unsigned int rid;
            public:
                typedef std::pair<std::string,std::string> ArgumentPair;
                typedef std::vector<ArgumentPair> ArgumentList;
                Response(std::string const &method, std::string const &resp, unsigned int rid)
                :method(method),rid(rid)
                {
                    std::istringstream rs(resp);
                    read_xml(rs,response);
                    boost::property_tree::ptree::const_assoc_iterator n = response.find("response");
                    if(n != response.not_found())
                    {
                        boost::property_tree::ptree::const_assoc_iterator ci =  n->second.find("clientInfo");
                        
                        if(ci != n->second.not_found())
                        {
                            if(ci->second.count("rid") && ci->second.get<unsigned int>("rid") == rid)
                                return;
                        }
                    }
                    throw (Exception("Invalid Response"));
                }
                
                std::string GetArgument(std::string const &arg) const
                {
                    boost::property_tree::ptree::const_assoc_iterator n = response.find("response");
                    if(n != response.not_found())
                    {
                        boost::property_tree::ptree::const_assoc_iterator mr = n->second.find(method+"Response");
                        if(mr != n->second.not_found())
                        {
                            std::vector<std::string> parts;
                            boost::split(parts,arg,boost::is_any_of("/"));
                            return mr->second.find( boost::join(parts,"."))->second.data();
                        }
                    }
                    return "";
                }
                
                ArgumentList GetArgumentList(std::string const &arg) const
                {
                    ArgumentList ret;
                    boost::property_tree::ptree::const_assoc_iterator n = response.find("response");
                    if(n != response.not_found())
                    {
                        boost::property_tree::ptree::const_assoc_iterator mr = n->second.find(method+"Response");
                        if(mr != n->second.not_found())
                        {
                            boost::property_tree::ptree::const_iterator part = n->second.to_iterator(mr);
                            while(part->second.begin() != part->second.end())
                            {
                                part = part->second.begin();
                                ret.push_back(ArgumentPair(part->first,part->second.data()));
                            }
                        }
                    }
                    return ret;
                }
                
                operator std::string() const
                {
                    std::stringstream ret;
                    write_xml(ret,response);
                    return ret.str();
                }
                
                int GetErrorCode() const
                {
                    response.get("response.fault.detail.errorcode",-999);
                }
                
                std::string GetErrorMessage() const
                {
                    return response.get("response.fault.detail.message",std::string("Error message not found in response!"));
                }
        };
    }
}

#endif
