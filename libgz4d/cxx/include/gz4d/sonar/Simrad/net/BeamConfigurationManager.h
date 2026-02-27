#ifndef GZ4D_SIMRAD_BEAMCONFIGURATIONMANAGER_H
#define GZ4D_SIMRAD_BEAMCONFIGURATIONMANAGER_H

#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>
#include <gz4d/sonar/Simrad/net/BeamConfiguration.h>

namespace gz4d
{
    namespace Simrad
    {
        /// Keeps track of ME70 beam configurations.
        class BeamConfigurationManager
        {
            ParameterManager::Ptr parameterManager;
            ParameterGroup::Map parameters;

            std::map<std::string,BeamConfiguration::Ptr > configs;

            void Subscribe()
            {
                if(!subscribed)
                {
                    parameters["manager"]->Add("active","BeamConfigurationMgr/ConfigurationNameSelected");
                    parameters["manager"]->Add("names","BeamConfigurationMgr/ConfigurationNames");
                    subscribed = true;
                }
            }
            
            bool subscribed;
            public:
                typedef std::shared_ptr<BeamConfigurationManager> Ptr;

                BeamConfigurationManager(ParameterManager::Ptr &p)
                :parameterManager(p),subscribed(false)
                {
                    parameters["manager"] = ParameterGroup::Ptr(new ParameterGroup(parameterManager));
                    
                    std::vector<std::string> configList;
                    try
                    {
                        std::string cl = parameterManager->Get("BeamConfigurationMgr/ConfigurationNames");
                        boost::algorithm::split(configList, cl, boost::algorithm::is_any_of(";"));
                        
                        for (std::vector<std::string>::iterator c = configList.begin(); c != configList.end(); ++c)
                        {
                            configs[*c] = BeamConfiguration::Ptr (new BeamConfiguration(parameterManager, parameters, *c));
                        }
                    }
                    catch(InvalidParameterException e)
                    {
                    }
                }
                
                ParameterGroup::Map const &GetParameters()
                {
                    Subscribe();
                    return parameters;
                }
                
                std::vector<BeamConfiguration::Ptr > GetConfigurations()
                {
                    Subscribe();
                    std::vector<gz4d::Simrad::BeamConfiguration::Ptr> ret;
                    for (std::map<std::string,BeamConfiguration::Ptr>::iterator c = configs.begin(); c != configs.end(); ++c)
                        ret.push_back(c->second);
                    return ret;
                }
                

        };
    }
}

#endif
