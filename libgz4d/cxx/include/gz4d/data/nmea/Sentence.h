#ifndef GZ4D_DATA_NMEA_SENTENCE_H
#define GZ4D_DATA_NMEA_SENTENCE_H

#include <gz4d/gz4d.h>
#include <gz4d/Exception.h>
#include <vector>
#include <memory>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp>

namespace gz4d
{
    namespace nmea
    {
        class Sentence
        {
            char prefix;
            std::vector<std::string> fields;
            std::string extras;

            public:
                typedef std::shared_ptr<Sentence> Ptr;

                Sentence()
                :prefix('$')
                {}
                
                Sentence(std::string const &s, bool strictValidation = true)
                {
                    Parse(s, strictValidation);
                }
                
                std::string const &getExtras() const {return extras;}

                void Parse(std::string const &s, bool strictValidation = true)
                {
                    fields.clear();
                    
                    std::string sentence = boost::algorithm::trim_copy(s);
                    int len = sentence.size();
                    if(len < 4)
                    {
                        throw Exception("Sentence not long enough");
                    }
                    if(sentence[0] != '$' && sentence[0] != '!')
                    {
                        if(strictValidation )
                            throw Exception("Leading $ not found");
                    }
                    
                    prefix = sentence[0];

                    std::size_t cs = sentence.find('*');
                    
                    if(strictValidation)
                    {
                        if( (cs == std::string::npos || cs > sentence.size()-3 ))
                            throw Exception("checksum not found");

                        extras = sentence.substr(cs+3);
                        
                        sentence = sentence.substr(0,cs+3);
                        len = sentence.size();
                        
                        if(GenerateChecksum(sentence) != sentence.substr(len-2))
                            throw Exception("invalid checksum");
                    }
                    
                    
                    if(sentence[len-3] == '*')
                        sentence = sentence.substr(1,len-4);
                    else
                        sentence = sentence.substr(1,len-1);
                    
                    boost::algorithm::split(fields,sentence,boost::algorithm::is_any_of(","));
                }
                
                
                static std::string GenerateChecksum(std::string const &s)
                {
                    char sum = 0;
                    for(int i = 0; i < s.size(); ++i)
                    {
                        if(i == 0 && (s[i] == '$' || s[i] == '!'))
                            ++i;
                        if(s[i] == '*')
                            break;
                        sum ^= s[i];
                    }
                    std::stringstream ssum;
                    ssum << std::hex << std::setw(2) << std::setfill('0') << int(sum);
                    return boost::algorithm::to_upper_copy(ssum.str());
                }
                

                std::size_t size() const {return fields.size();}
                void resize(size_t s){fields.resize(s);}
                bool empty() const {return fields.empty();}
                std::string const &operator[](unsigned int index) const {return fields[index];}
                std::string &operator[](unsigned int index) {return fields[index];}

                std::string operator()() const
                {
                    std::stringstream ret;
                    ret << prefix;
                    for(std::vector<std::string>::const_iterator s = fields.begin(); s != fields.end(); ++s)
                    {
                        if (s != fields.begin())
                            ret << ",";
                        ret << *s;
                    }
                    ret << "*" << GenerateChecksum(ret.str());
                    ret << char(13) << char(10);
                    return ret.str();
                }
                

                bool IsType(std::string const &pattern) const
                {
                    if (fields.empty() || fields[0].size() != pattern.size())
                        return false;
                    
                    for(int i = 0; i < pattern.size(); ++i)
                    {
                        if(pattern[i] != '?' && pattern[i] != fields[0][i])
                            return false;
                    }
                    return true;
                }
                

        };

    }
}

#endif
