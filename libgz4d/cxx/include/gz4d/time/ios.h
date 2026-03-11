// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_TIME_IOS_H
#define GZ4D_TIME_IOS_H

#include "../time.h"

namespace gz4d
{
    namespace time
    {
        template <class charT, class traits> inline std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& str,Point const &t)
        {
            if(t.valid())
            {
                gz4d::time::Parts p = t.decompose();
                int saved_precision = str.precision();
                std::ios::fmtflags saved_flags = str.flags();
                str << p.year << "-" << std::setfill('0') << std::setw(2) << (int)(p.month) << "-" << std::setw(2) << (int)(p.day) << "_" << std::setw(2) << (int)(p.hour) << "." << std::setw(2) << (int)(p.minute) << "." << std::fixed << std::setw(6) << std::setprecision(3) << p.second+p.frac_second;
                str.precision(saved_precision);
                str.setf(saved_flags);
                str.unsetf(~saved_flags);
            }
            else
                str << "(Invalid Time)";

            return str;
        }


        template <class charT, class traits> inline std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& str, gz4d::time::Point &t)
        {
            std::string ds;
            std::string ts;
            str >> ds;
            if(ds.size() > 11)
                ts = ds.substr(11);
            else
                str >> ts;
            
            uint64_t year = boost::lexical_cast<uint64_t>(ds.substr(0,4));

            std::string temp_str = ds.substr(5,2);
            if(temp_str[0] == '0')
                temp_str = temp_str.substr(1);
            unsigned int month = boost::lexical_cast<unsigned int>(temp_str);

            temp_str = ds.substr(8,2);
            if(temp_str[0] == '0')
                temp_str = temp_str.substr(1);
            unsigned int day = boost::lexical_cast<unsigned int>(temp_str);

            temp_str = ts.substr(0,2);
            if(temp_str[0] == '0')
                temp_str = temp_str.substr(1);
            unsigned int hour = boost::lexical_cast<unsigned int>(temp_str);

            temp_str = ts.substr(3,2);
            if(temp_str[0] == '0')
                temp_str = temp_str.substr(1);
            unsigned int minute = boost::lexical_cast<unsigned int>(temp_str);

            temp_str = ts.substr(6);
            if(temp_str[0] == '0')
                temp_str = temp_str.substr(1);
            double second = boost::lexical_cast<double>(temp_str);
            t = gz4d::time::Point(year,month,day,hour,minute,second);
            return str;
        }


        template <class charT, class traits> inline std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& str, gz4d::time::Duration const &d)
        {
            return str << std::string(d);
        }

        template <class charT, class traits> inline std::basic_istream<charT,traits>& operator>>(std::basic_istream<charT,traits>& str, gz4d::time::Duration &d)
        {
            std::string ds;
            str >> ds;
            d = gz4d::time::Duration(ds);
            return str;
        }
    }
}


#endif
