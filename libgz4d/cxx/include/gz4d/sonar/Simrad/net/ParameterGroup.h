// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_PARAMETERGROUP_H
#define GZ4D_SIMRAD_PARAMETERGROUP_H

#include <gz4d/sonar/Simrad/net/ParameterManager.h>

namespace gz4d
{
    namespace Simrad
    {
        class ParameterGroup: public Producer<gz4d::Time>, public Consumer<ParameterGroup, gz4d::Time>

        {
            ParameterManager::Ptr manager;
            gz4d::Time lastTime;

            protected:
                std::map<std::string,Parameter::Ptr> params;
                boost::mutex paramsMutex;

            public:
                typedef std::shared_ptr<ParameterGroup> Ptr;
                typedef std::map<std::string,ParameterGroup::Ptr> Map;

                ParameterGroup(std::shared_ptr<ParameterManager> &pm)
                :manager(pm)
                {
                }
                

                bool Add(std::string const &l, std::string const &p, bool useDefault = true)
                {
                    boost::mutex::scoped_lock lock(paramsMutex);
                    try
                    {
                        params[l] = manager->Subscribe(p,useDefault);
                    }
                    catch(InvalidParameterException e)
                    {
                        std::cerr << "ParameterGroup::Add " << e.what() << std::endl;
                        return false;
                    }
                    params[l]->AddOutQueue(GetInQueue());
                    return true;
                }
                

                void Consume(gz4d::Time t)
                {
                    if(!lastTime.valid() && t.valid())
                        lastTime = t;
                    
                    if(t > lastTime)
                    {
                        Send(lastTime);
                        lastTime = t;
                    }
                    
                }
                
                Parameter::Ptr operator[](std::string const &key)
                {
                    boost::mutex::scoped_lock lock(paramsMutex);
                    if(params.count(key))
                        return params[key];
                    return Parameter::Ptr();
                }
                
                
        };
    }
}

#endif
