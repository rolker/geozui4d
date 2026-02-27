#ifndef GZ4D_SIMRAD_TARGETDETECTION_H
#define GZ4D_SIMRAD_TARGETDETECTION_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/Target.h>

namespace gz4d
{
    namespace Simrad
    {

        class TargetDetectionSubscription: public TargetSubscription
        {
            public:
                typedef std::shared_ptr<TargetDetectionSubscription> Ptr;
                TargetDetectionSubscription(std::string channel, ParameterGroup::Map const &pingParams)
                :TargetSubscription("TSDetection",channel,pingParams)
                {
                }
                
                void AddData(std::shared_ptr<std::vector<unsigned char> > &data)
                {
                    #pragma pack(1)
                    struct Header
                    {
                        uint64_t time;
                        uint16_t count;
                    };
                    
                    struct EchoTrace
                    {
                        double depth;
                        double tsComp;
                        double tsUncomp;
                        double alongshipAngle;
                        double athwartshipAngle;
                        double sa;
                    };
                    #pragma pack()
                    
                    if(data->size())
                    {
                        Header * h = reinterpret_cast<Header *>(&data->front());
                        if(h->count)
                        {
                            EchoTrace * e = reinterpret_cast<EchoTrace *>(&h[1]);
                            
                            TargetSet ts;
                            ts.time = gz4d::time::FromNT(h->time);
                            
                            double dirX = 0.0;
                            if((*pingParameters["channel"])["directionX"])
                                dirX = (*pingParameters["channel"])["directionX"]->Get<double>(Nan<double>(),ts.time);
                            double dirY = 0.0;
                            if((*pingParameters["channel"])["directionY"])
                                dirY = (*pingParameters["channel"])["directionY"]->Get<double>(Nan<double>(),ts.time);
                            
                            for(int i = 0; i < h->count; ++i)
                            {
                                Target t;
                                t.position[0] = e[i].depth*tan(Radians(dirY));
                                t.position[1] = e[i].depth*tan(Radians(dirX));
                                t.position[2] = -e[i].depth;
                                //std::cerr << i << "\tdepth: " << -e[i].depth << "\t" << t.position << std::endl;
                                t.value = e[i].sa;
                                ts.targets.push_back(t);
                                //std::cerr << i << "\t" << ts.targets.back().position << std::endl;
                            }
                            Send(ts);
                        }
                    }
                }
                
        };
    }
}

#endif
