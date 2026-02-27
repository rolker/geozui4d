#ifndef GZ4D_SIMRAD_H
#define GZ4D_SIMRAD_H

#include <gz4d/sonar/Sonar.h>
#include <gz4d/Exception.h>

#include <map>
#include <cmath>
#include <vector>
//#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>


namespace gz4d
{
    /// Interface to Simrad ME70 Multibeam Sonar
    namespace Simrad
    {
        class GZ4DEXPORT Exception: public gz4d::Exception
        {
            public:
                Exception(std::string msg):gz4d::Exception(msg){}
        };

        class GZ4DEXPORT AccessDeniedException: public Exception
        {
            public:
                AccessDeniedException(std::string msg):Exception(msg){}
        };

        class GZ4DEXPORT InvalidParameterException: public Exception
        {
            public:
                InvalidParameterException(std::string msg):Exception(msg){}
        };

        class ParameterMap: public std::map<std::string,std::string>
        {
            std::string joiner;
            public:
                ParameterMap():joiner(","){}
                ParameterMap(std::string const &s)
                :joiner(",")
                {
                    std::vector<std::string> parts;
                    boost::split(parts,s,boost::is_any_of(","));
                    for(std::vector<std::string>::iterator i = parts.begin(); i != parts.end(); ++i)
                    {
                        std::string::size_type sep = i->find(':');
                        if (sep == std::string::npos)
                            throw Exception("Invalid parameter string");
                        
                        std::string key(*i,0,sep);
                        std::string val(*i,sep+1);
                        boost::trim(key);
                        boost::trim(val);
                        insert(value_type(key,val));
                    }
                }
                
                operator std::string() const
                {
                    std::string ret;
                    for(const_iterator i = begin(); i != end(); ++i)
                    {
                        if(!ret.empty())
                            ret += joiner;
                        ret += i->first + ":" + i->second;
                    }
                    return ret;
                }
                
                void SetJoiner(std::string const &j){joiner = j;}
        };

        inline float RawToDB(int16_t r){return r*0.01175898420562426544;}

    }
}

#endif
