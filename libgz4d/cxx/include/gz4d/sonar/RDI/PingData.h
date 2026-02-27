#ifndef GZ4D_SONAR_RDI_PINGDATA_H
#define GZ4D_SONAR_RDI_PINGDATA_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/PingData.h>
#include <gz4d/sonar/RDI/ChannelData.h>
#include <gz4d/sonar/RDI/Velocities.h>
#include <gz4d/sonar/Ping.h>
#include <gz4d/nav/Fix.h>
#include <gz4d/util/Swap.h>


namespace gz4d
{
    namespace RDI
    {
        class PingData: public Data, public sonar::PingData<RDI::Channel>
        {
            public:
                PingData(EnsembleWrapper ensemble, int year, gz4d::Time t)
                :Data(ensemble,year,t)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        ChannelData::Ptr cd(new ChannelData(ensemble, year, i, t));
                        channels.push_back(Channel(cd));
                    }
                }
                

                Velocities GetVelocities()
                {
                    return Velocities(*this);
                }
                
                Velocities GetVelocities(float heading, float course, float speed)
                {
                    return Velocities(*this,heading,course,speed);
                }
                
                double GetTransducerDepth()
                {
                    if(ensemble.old())
                        return Nan<double>();
                    return ensemble.GetData<VariableLeader>()->DepthOfTransducer*0.1;
                }
                
        };

        typedef sonar::Ping<PingData,nav::Fix> Ping;
    }
}

#endif
