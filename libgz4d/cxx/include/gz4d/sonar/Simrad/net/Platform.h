#ifndef GZ4D_SIMRAD_PLATFORM_H
#define GZ4D_SIMRAD_PLATFORM_H

#include <gz4d/sonar/Simrad/net/ParameterManager.h>
#include <gz4d/sonar/Simrad/net/ParameterGroup.h>

namespace gz4d
{
    namespace Simrad
    {
        /// Keeps track of data pertaining to the platform.
        /// Data includes navigation, which is time varying
        /// and offsets, which should be static.
        class Platform
        {
            ParameterManager::Ptr parameterManager;
            ParameterGroup::Map parameters;

            void Subscribe()
            {
                if(!subscribed)
                {
                    parameters["position"] = ParameterGroup::Ptr(new ParameterGroup(parameterManager));
                    parameters["position"]->Add("lat","OwnShip/Latitude",false);
                    parameters["position"]->Add("lon","OwnShip/Longitude",false);
                    
                    parameters["heading"] = ParameterGroup::Ptr(new ParameterGroup(parameterManager));
                    parameters["heading"]->Add("heading","OwnShip/Heading",false);
                    
                    parameters["attitude"] = ParameterGroup::Ptr(new ParameterGroup(parameterManager));
                    parameters["attitude"]->Add("roll","OwnShip/Roll");
                    parameters["attitude"]->Add("pitch","OwnShip/Pitch");
                    parameters["attitude"]->Add("heave","OwnShip/Heave");
                    subscribed = true;
                }
            }
            
            bool subscribed;
            public:
                typedef std::shared_ptr<Platform> Ptr;

                Platform(ParameterManager::Ptr &p)
                :parameterManager(p),subscribed(false)
                {}
                

                ParameterGroup::Map const &GetParameters()
                {
                    Subscribe();
                    return parameters;
                }
                
        };
    }
}

#endif
