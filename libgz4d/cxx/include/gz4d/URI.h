// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_URI_H
#define GZ4D_URI_H

#include "gz4d.h"
#include "Exception.h"
#include "util/string.h"

#include <string>
#include <sstream>
#include <map>

namespace gz4d
{
    /// Handles Uniform Resource Identifiers.
    /// Decomposes a URI (or URL) into its parts
    /// or assembles the parts into a URI.
    ///
    /// see rfc 3986

    class URIException: public Exception
    {
    public:
        URIException(const std::string& part, const std::string &bad_data, const std::string &uri)
        :Exception("Invalid "+part+": "+bad_data+" in URI: "+uri){}
        URIException(const std::string& part, const std::string &uri)
        :Exception("Invalid "+part+" in URI: "+uri){}
        
    };
    
    struct URI
    {
        std::string scheme;    ///< Also known as the protocol

        std::string userinfo;
        std::string host; 
        int port;             ///< Negative if not specified
        
        
        std::string path;
        std::string query;
        std::string fragment;
        
        
        URI(URI const &uri):scheme(uri.scheme),userinfo(uri.userinfo),
            host(uri.host),port(uri.port),path(uri.path),query(uri.query),fragment(uri.fragment)
        {
        }

        /// Parse a string containing a URI.
        /// Does not validate the results, assumes reasonable input!
        URI(std::string const &uri=std::string("")):port(-1)
        {
            if(uri.empty())
                return;
            std::string rest = uri;
            std::size_t scheme_end = rest.find(":");
            if(scheme_end == std::string::npos)
                throw URIException("scheme",uri);
            scheme = lower(uri.substr(0,scheme_end));
            rest = rest.substr(scheme_end+1);
            
            if(rest.substr(0,2) == "//")
            {
                rest = rest.substr(2);
                std::size_t auth_end = rest.find_first_of("/?#");
                std::string authority;
                if(auth_end == std::string::npos)
                {
                    authority = rest;
                    rest = "";
                }
                else
                {
                    authority = rest.substr(0,auth_end);
                    rest = rest.substr(auth_end);
                }
                
                std::size_t uinfo_end = authority.find("@");
                if(uinfo_end != std::string::npos)
                {
                    userinfo = authority.substr(0,uinfo_end);
                    authority = authority.substr(uinfo_end+1);
                }
                
                if(!authority.empty() && authority[0] == '[')
                {
                    // IP6 or later host name
                    std::size_t host_end = authority.find("]");
                    if(host_end == std::string::npos)
                        throw URIException("host",authority,uri);
                    host = authority.substr(0,host_end+1);
                    authority = authority.substr(host_end+1);
                }
                else
                {
                    std::size_t host_end = authority.find(":");
                    if(host_end == std::string::npos)
                    {
                        host = authority;
                        authority = "";
                    }
                    else
                    {
                        host = authority.substr(0,host_end);
                        authority = authority.substr(host_end);
                    }
                }
                
                if(!authority.empty())
                {
                    if(authority[0] != ':')
                        throw URIException("port",authority,uri);
                    // allow for empty port with : following host
                    if(authority.size()>1)
                    {
                        port = atoi(authority.substr(1).c_str());
                        std::stringstream check;
                        check << port;
                        if (check.str() != authority.substr(1))
                            throw URIException("port",authority,uri);
                    }
                }
            }
            
            std::size_t path_end = rest.find_first_of("?#");
            if(path_end == std::string::npos)
            {
                path = rest;
                rest = "";
            }
            else
            {
                path = rest.substr(0,path_end);
                rest = rest.substr(path_end);
            }

            if(!rest.empty() && rest[0] == '?')
            {
                std::size_t q_end = rest.find("#");
                if(q_end == std::string::npos)
                {
                    query = rest.substr(1);
                    rest = "";
                }
                else
                {
                    query = rest.substr(1,q_end-1);
                    rest = rest.substr(q_end);
                }
            }
            
            if(!rest.empty())
            {
                if(rest[0] != '#')
                    throw URIException("fragment",rest,uri);
                fragment = rest.substr(1);
            }
        }
        
        operator std::string() const
        {
            std::stringstream ret;
            if(!scheme.empty())
                ret << scheme << ":";
            if(!host.empty())
            {
                ret << "//";
                if(!userinfo.empty())
                    ret << userinfo << "@";
                ret << host;
                if(port >=0)
                    ret << ":" << port;
            }
            
            ret << path;
            if(!query.empty())
                ret << "?" << query;
            if(!fragment.empty())
                ret << "#" << fragment;
            return ret.str();
        }
 
        /// Returns specified port, or default port for some known schemes when port not specified.
        int getPort() const
        {
            if(port == -1)
            {
                if(scheme == "ftp")
                    return 21;
                if(scheme == "ssh")
                    return 22;
                if(scheme == "telnet")
                    return 23;
                if(scheme == "gopher")
                    return 70;
                if(scheme == "http")
                    return 80;
                if(scheme == "ws")
                    return 80;
                if(scheme == "pop")
                    return 110;
                if(scheme == "nntp")
                    return 119;
                if(scheme == "news")
                    return 119;
                if(scheme == "imap")
                    return 143;
                if(scheme == "snmp")
                    return 161;
                if(scheme == "https")
                    return 443;
                if(scheme == "wss")
                    return 443;
                if(scheme == "afp")
                    return 548;
                if(scheme == "rtsp")
                    return 554;
                if(scheme == "rtspu")
                    return 554;
                if(scheme == "nntps")
                    return 563;
                if(scheme == "rsync")
                    return 873;
                if(scheme == "ircs")
                    return 994;
                if(scheme == "nfs")
                    return 2049;
                if(scheme == "sip")
                    return 5060;
                if(scheme == "sips") // or sip using TLS over TCP
                    return 5061;
                if(scheme == "irc")
                    return 6667;
                if(scheme == "git")
                    return 9418;
            }
            return port;
        }
        
        typedef  std::map<std::string,std::string> QueryMap;
        
        QueryMap getQueryMap(std::string const &delimiter=std::string(";&")) const
        {
            QueryMap ret;
            if(query.empty())
                return ret;
            std::string rest = query;
            while(!rest.empty())
            {
                std::size_t item_end = rest.find_first_of(delimiter);
                std::string item;
                if(item_end == std::string::npos)
                {
                    item = rest;
                    rest = "";
                }
                else
                {
                    item = rest.substr(0,item_end);
                    rest = rest.substr(item_end+1);
                }
                std::size_t key_end = item.find("=");
                if(key_end == std::string::npos)
                    ret[item]="";
                else
                    ret[item.substr(0,key_end)]=item.substr(key_end+1);
            }
            return ret;
        }
        
        static std::string encode(std::string const&)
        {
        }
    };
}

#endif

