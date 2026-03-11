// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_PARAMETER_H
#define GZ4D_SIMRAD_PARAMETER_H

#include <gz4d/sonar/Simrad.h>
#include <gz4d/sonar/Simrad/net/ParameterUpdate.h>
#include <queue>
#include <boost/thread/mutex.hpp>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <gz4d/thread/Producer.h>

namespace gz4d
{
    namespace Simrad
    {
        class Parameter: public Producer<gz4d::Time>
        {
            public:
                typedef std::shared_ptr<Parameter> Ptr;
                typedef std::weak_ptr<Parameter> WeakPtr;

                struct Info
                {
                    std::string name;
                    std::string description;
                    int type;
                    std::string defaultValue;
                };

                Parameter(Info info, bool useDefault = true, bool vIsTime = false){}

                void Update(ParameterUpdate const &update)
                {
                    gz4d::Time t = update.time;
                    if(valueIsTime)
                        t = gz4d::time::FromNT(update.value.operator()<uint64_t>());
                    {
                        boost::mutex::scoped_lock lock(updatesMutex);
                        updates[t] = update.value;
                        //std::cerr << "Parameter::Update\t" << update.id << "\t" << update.time << std::endl;
                    }
                    Send(t);
                }
                

                template<typename T> T Get(T nullVal, gz4d::Time time = gz4d::Time());
                template<typename T> bool TypeCheck();

                Info const &GetInfo() const {return info;}
                gz4d::Time GetLatestTime()
                {
                    boost::mutex::scoped_lock lock(updatesMutex);
                    if(!updates.empty())
                        return updates.rbegin()->first;
                }
                
            private:
                Info info;
                bool useDefault;
                bool valueIsTime;

                std::map<gz4d::Time,ParameterUpdate::Value> updates;
                boost::mutex updatesMutex;

        };

        template<typename T> inline T Parameter::Get(T nullVal, gz4d::Time time)
        {
            if(!TypeCheck<T>())
                throw(Exception("Type mismatch: " + boost::lexical_cast<std::string>(info.type)));

            {
                boost::mutex::scoped_lock lock(updatesMutex);
                if(!updates.empty())
                {
                    if(time.valid())
                    {
                        if(updates.begin()->first <= time)
                        {
                            if (updates.count(time))
                                return updates[time].operator()<T>();
                            std::map<gz4d::Time,ParameterUpdate::Value>::iterator i = updates.upper_bound(time);
                            if (i != updates.end())
                            {
                                --i;
                                return i->second.operator()<T>();
                            }
                            return updates.rbegin()->second.operator()<T>();
                        }
                    }
                    else
                        return updates.rbegin()->second.operator()<T>();
                }
            }
            if(useDefault && !info.defaultValue.empty())
                return boost::lexical_cast<T>(info.defaultValue);
            return nullVal;
        }


        template<> inline bool Parameter::TypeCheck<short>()
        {
            return (info.type==2);
        }

        template<> inline bool Parameter::TypeCheck<int>()
        {
            return (info.type==3);
        }

        template<> inline bool Parameter::TypeCheck<int64_t>()
        {
            return (info.type==20);
        }

        template<> inline bool Parameter::TypeCheck<float>()
        {
            return (info.type==4);
        }

        template<> inline bool Parameter::TypeCheck<double>()
        {
            return (info.type==5);
        }

        template<> inline bool Parameter::TypeCheck<uint64_t>()
        {
            return (info.type==6);
        }

        template<> inline bool Parameter::TypeCheck<bool>()
        {
            return (info.type==11);
        }

        template<> inline bool Parameter::TypeCheck<std::string>()
        {
            return (info.type==8);
        }

        template<> inline bool Parameter::TypeCheck<std::vector<double> >()
        {
            return (info.type==0x2005);
        }

        template<> inline bool Parameter::TypeCheck<std::vector<std::string> >()
        {
            return (info.type==0x2008);
        }

    }
}


#endif
