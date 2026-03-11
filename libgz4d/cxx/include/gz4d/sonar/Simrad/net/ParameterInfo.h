// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_PARAMETERINFO_H
#define GZ4D_SIMRAD_PARAMETERINFO_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/ParameterUpdate.h>

namespace gz4d
{
    namespace Simrad
    {
        class GZ4DEXPORT ParameterInfo
        {
            protected:
                std::string name;
                int id;
                int type;
            public:
                ParameterInfo(std::string n):name(n){}
                ParameterInfo(ParameterInfo const &pi):name(pi.name),id(pi.id),type(pi.type){}
                ParameterInfo(){}

                void SetID(int i){id = i;}
                int GetID() const {return id;}

                std::string const &GetName() const {return name;}

                void SetType(int t){type = t;}
                int GetType() const {return type;}

                template<typename T>
                T Cast(ParameterUpdate::Value const &v) const;
        };

        template<typename T>
                inline T ParameterInfo::Cast(ParameterUpdate::Value const &v) const
        {
            return *reinterpret_cast<T const*>(&v.front());
        }

        template<>
                inline std::string ParameterInfo::Cast(ParameterUpdate::Value const &v) const
        {
            return std::string(v.begin(),v.end());
        }

        template<>
                inline std::vector<std::string> ParameterInfo::Cast(ParameterUpdate::Value const &v) const
        {
            std::vector<std::string> ret;
            char const *cursor = &v.front();
            while(cursor != &(*v.end()))
            {
                ret.push_back(cursor);
                cursor = &cursor[ret.back().size()+1];
            }
            return ret;
        }

        template<>
                inline std::vector<double> ParameterInfo::Cast(ParameterUpdate::Value const &v) const
        {
            std::vector<double> ret(reinterpret_cast<double const *>(&(*v.begin())),reinterpret_cast<double const *>(&(*v.end())));
            return ret;
        }

    }
}

#endif
