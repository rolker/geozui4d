// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_PARAMETERUPDATE_H
#define GZ4D_SIMRAD_PARAMETERUPDATE_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/time.h>
#include <vector>

namespace gz4d
{
    namespace Simrad
    {

        struct GZ4DEXPORT ParameterUpdate
        {
            struct Value: public std::vector<char>
            {
                using std::vector<char>::operator=;
                template<typename T>
                        T operator()() const;
            };

            int id;
            Value value;
            gz4d::Time time;
        };

        template<typename T>
                inline T ParameterUpdate::Value::operator()() const
        {
            return *reinterpret_cast<T const*>(&this->front());
        }

        template<>
                inline std::string ParameterUpdate::Value::operator()() const
        {
            return std::string(this->begin(),this->end());
        }

        template<>
                inline std::vector<std::string> ParameterUpdate::Value::operator()() const
        {
            std::vector<std::string> ret;
            char const *cursor = &this->front();
            while(cursor != &(*this->end()))
            {
                ret.push_back(cursor);
                cursor = &cursor[ret.back().size()+1];
            }
            return ret;
        }

        template<>
                inline std::vector<double> ParameterUpdate::Value::operator()() const
        {
            double const *ret = reinterpret_cast<double const*>(&this->front());
            return std::vector<double>(ret,&ret[this->size()/sizeof(double)]);

        }

    }
}

#endif
