// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_BOTTOMDETECTION_H
#define GZ4D_SIMRAD_BOTTOMDETECTION_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/Target.h>

namespace gz4d
{
    namespace Simrad
    {
        class BottomDetectionSubscription: public TargetSubscription
        {
            public:
                typedef std::shared_ptr<BottomDetectionSubscription> Ptr;

                BottomDetectionSubscription(std::string channel, ParameterGroup::Map const &pingParams)
                :TargetSubscription("BottomDetection",channel, pingParams)
                {
                }
                
                void AddData(std::shared_ptr<std::vector<unsigned char> > &data)
                {
                    #pragma pack(1)
                    struct BottomDepth
                    {
                        uint64_t time;
                        double bottomDepth;
                        double reflectivity;
                    };
                    #pragma pack()
                    
                    if(data->size())
                    {
                        BottomDepth * b = reinterpret_cast<BottomDepth *>(&data->front());
                        TargetSet bd;
                        bd.time = gz4d::time::FromNT(b->time);
                        
                        
                        double dirX = 0.0;
                        if ((*pingParameters["channel"])["directionX"])
                        {
                            dirX = (*pingParameters["channel"])["directionX"]->Get<double>(Nan<double>(),bd.time);
                        }
                        
                        double dirY = 0.0;
                        if ((*pingParameters["channel"])["directionY"])
                        {
                            dirY = (*pingParameters["channel"])["directionY"]->Get<double>(Nan<double>(),bd.time);
                        }
                        
                        double heading = (*pingParameters["navigation"])["heading"]->Get<double>(0.0,bd.time);
                        
                        double x = -b->bottomDepth*sin(Radians(dirY));
                        double y = -b->bottomDepth*sin(Radians(dirX));
                        
                        bd.targets.resize(1);
                        bd.targets[0].position[0] = cos(-heading)*x + sin(-heading)*y;
                        bd.targets[0].position[1] = cos(-heading)*y + sin(-heading)*x;
                        bd.targets[0].position[2] = -b->bottomDepth*cos(Radians(dirY));
                        bd.targets[0].value = b->reflectivity;
                        Send(bd);
                    }
                    
                }
                
        };
    }
}

#endif
