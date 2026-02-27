#ifndef GZ4D_SIMRAD_BEAMCONFIGURATION_H
#define GZ4D_SIMRAD_BEAMCONFIGURATION_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>

namespace gz4d
{
    namespace Simrad
    {
        class BeamConfiguration
        {
            ParameterManager::Ptr parameterManager;
            ParameterGroup::Map parameters;

            std::string name;
            void Subscribe()
            {
                if(!subscribed)
                {
                    parameters["configuration"]->Add("description","BeamConfigurationMgr/"+name+"/ConfigurationDescription");
                    parameters["configuration"]->Add("frequencyUpper","BeamConfigurationMgr/"+name+"/FrequencyUpper");
                    parameters["configuration"]->Add("frequencyLower","BeamConfigurationMgr/"+name+"/FrequencyLower");
                    parameters["configuration"]->Add("beamCount","BeamConfigurationMgr/"+name+"/NoOfBeamsInFan");
                    subscribed = true;
                }
            }
            
            bool subscribed;
            public:
                typedef std::shared_ptr<BeamConfiguration> Ptr;

                BeamConfiguration(ParameterManager::Ptr &p, ParameterGroup::Map const &params, std::string const &name)
                :parameterManager(p),parameters(params),name(name)
                {
                    parameters["configuration"] = ParameterGroup::Ptr(new ParameterGroup(p));
                }
                

                std::string GetName(){return name;}

                ParameterGroup::Map const &GetParameters()
                {
                    Subscribe();
                    return parameters;
                }
                
        };
    }
}

#endif
