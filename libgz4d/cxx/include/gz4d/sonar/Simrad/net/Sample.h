// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SIMRAD_SAMPLE_H
#define GZ4D_SIMRAD_SAMPLE_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/Target.h>
#include <gz4d/sonar/Simrad/net/Sample.h>
#include <gz4d/Rotation.h>

namespace gz4d
{
    namespace Simrad
    {

        struct SampleSet
        {
            gz4d::Time time;
            float z1;
            float sampleInterval;
            float soundSpeed;
            std::vector<int16_t> samples;
            typedef std::shared_ptr<SampleSet> Ptr;
        };

        class SampleSubscription: public Subscription, public Producer<SampleSet::Ptr>
        {
            std::list<SampleSet> samples;
            boost::mutex samplesMutex;
            public:
                typedef std::shared_ptr<SampleSubscription> Ptr;
                SampleSubscription(std::string channel, ParameterGroup::Map const &pingParams)
                :Subscription("SampleData",channel,pingParams)
                {
                }
                

                void AddData(std::shared_ptr<std::vector<unsigned char> > &data)
                {
                    #pragma pack(1)
                    struct SampleData
                    {
                        uint64_t time;
                        int16_t data[1];
                    };
                    #pragma pack()
                    
                    if(data->size())
                    {
                        SampleData * d = reinterpret_cast<SampleData *>(&data->front());
                        
                        double interval = (*pingParameters["channel"])["sampleInterval"]->Get<double>(Nan<double>());
                        double sspeed = (*pingParameters["channel"])["soundSpeed"]->Get<double>(Nan<double>());
                        double depth = 0.0;
                        if((*pingParameters["channel"])["transducerDepth"])
                            depth = (*pingParameters["channel"])["transducerDepth"]->Get<double>(depth);
                        
                        double dirX = 0.0;
                        if((*pingParameters["channel"])["directionX"])
                            dirX = (*pingParameters["channel"])["directionX"]->Get<double>(Nan<double>());
                        
                        double dirY = 0.0;
                        if((*pingParameters["channel"])["directionY"])
                            dirY = (*pingParameters["channel"])["directionY"]->Get<double>(Nan<double>());
                        
                        Rotation<double> roll (-Angle<double,Radian>(dirX),Point<double>(0.0,1.0,0.0));
                        Rotation<double> pitch (Angle<double,Radian>(dirY),Point<double>(1.0,1.0,0.0));
                        
                        Point<double> unitVec = pitch(roll(Point<double>(0.0,0.0,interval*sspeed)));
                        
                        SampleSet::Ptr ss(new SampleSet);
                        ss->time = gz4d::time::FromNT(d->time);
                        ss->z1 = -depth;
                        ss->sampleInterval = interval;
                        ss->soundSpeed = sspeed;
                        
                        int count = (data->size()-sizeof(uint64_t))/sizeof(uint16_t);
                        
                        for(int i = 0; i < count; ++i)
                        {
                            ss->samples.push_back(d->data[i]);
                        }
                        Send(ss);
                    }
                }
                    
        };
    }
}

#endif
