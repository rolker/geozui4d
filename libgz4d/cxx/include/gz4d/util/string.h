#ifndef GZ4D_UTIL_STRING_H
#define GZ4D_UTIL_STRING_H

#include "../gz4d.h"
#include <string>
#include <sstream>
#include <cctype>

namespace gz4d
{
    inline std::string upper(std::string const &s)
    {
        std::stringstream ret;
        for (std::string::const_iterator c = s.begin(); c != s.end(); ++c)
            ret << char(toupper(*c));
        return ret.str();
    }
    
    inline std::string lower(std::string const &s)
    {
        std::stringstream ret;
        for (std::string::const_iterator c = s.begin(); c != s.end(); ++c)
            ret << char(tolower(*c));
        return ret.str();
    }
    
}

#endif

