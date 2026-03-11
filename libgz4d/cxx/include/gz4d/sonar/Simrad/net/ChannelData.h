// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_SIMRAD_NET_CHANNELDATA_H
#define GZ4D_SONAR_SIMRAD_NET_CHANNELDATA_H

#include <gz4d/sonar/Simrad/Sample.h>
#include <gz4d/sonar/Simrad/net/Sample.h>
#include <gz4d/sonar/Channel.h>
#include <gz4d/sonar/Ping.h>
#include <gz4d/time.h>
#include <gz4d/nav/Fix.h>

namespace gz4d
{
    namespace Simrad
    {
        namespace net
        {
            class ChannelData
            {
                SampleSet::Ptr samples;

                float totalTransmitDuration;

                uint16_t cid;
                float G0; ///< Peak gain
                float Sa;
                float equivalentBeamAngle;
                float frequency;
                float soundSpeed;
                float pulseDuration;
                float absorptionCoefficient;
                float transmitPower;
                float sampleInterval;
                sonar::BeamGeometry beamGeometry;

                public:
                    typedef std::shared_ptr<ChannelData> Ptr;
                    typedef SampleAdaptors<ChannelData> functor_types;
                    typedef int16_t* iterator;

                    ChannelData(){}
                    ChannelData(ParameterGroup::Map &params, SampleSet::Ptr & samples, float ttd, uint16_t cid)
                    :samples(samples),totalTransmitDuration(ttd),cid(cid)
                    {
                        G0 = (*params["channel"])["gain"]->Get<double>(Nan<double>());
                        Sa = 0.0;
                        absorptionCoefficient = (*params["channel"])["absorptionCoefficient"]->Get<double>(Nan<double>());
                        equivalentBeamAngle = (*params["channel"])["equivalentBeamAngle"]->Get<double>(Nan<double>());
                        transmitPower = (*params["channel"])["transmitPower"]->Get<double>(Nan<double>());
                        frequency = (*params["channel"])["frequency"]->Get<double>(Nan<double>());
                        soundSpeed = (*params["channel"])["soundSpeed"]->Get<double>(Nan<double>());
                        pulseDuration  = (*params["channel"])["pulseLength"]->Get<double>(Nan<double>());
                        sampleInterval  = (*params["channel"])["sampleInterval"]->Get<double>(Nan<double>());
                        
                        if ((*params["channel"])["directionX"])
                        {
                            beamGeometry.direction[0] = (*params["channel"])["directionX"]->Get<double>(Nan<double>());
                        }
                        else
                            beamGeometry.direction[0] = 0.0;
                        
                        if ((*params["channel"])["directionY"])
                        {
                            beamGeometry.direction[1] = (*params["channel"])["directionY"]->Get<double>(Nan<double>());
                        }
                        else
                            beamGeometry.direction[1] = 0.0;
                        
                        beamGeometry.width[0] = (*params["channel"])["beamWidthX"]->Get<double>(Nan<double>());
                        beamGeometry.width[1] = (*params["channel"])["beamWidthY"]->Get<double>(Nan<double>());
                        
                    }
                    

                    iterator begin()
                    {
                        return &samples->samples.front();
                    }
                    
                    iterator end()
                    {
                        return &(*(samples->samples.end()));
                    }
                    

                    uint16_t ID() const
                    {
                        return cid;
                    }
                    
                    gz4d::Time Timestamp() const
                    {
                        return samples->time;
                    }
                    
                    float TransmitPower() const
                    {
                        return transmitPower;
                    }
                    
                    float PeakGain() const {return G0;}
                    float Frequency() const
                    {
                        return frequency;
                    }
                    
                    float SoundSpeed() const
                    {
                        return soundSpeed;
                    }
                    
                    float PulseDuration() const
                    {
                        return pulseDuration;
                    }
                    
                    float AbsorptionCoefficient() const
                    {
                        return absorptionCoefficient;
                    }
                    
                    float SaCorrection() const {return Sa;}
                    float SampleInterval() const
                    {
                        return sampleInterval;
                    }
                    
                    float SampleOffset() const
                    {
                        return 0.0;
                    }
                    
                    float EquivalentBeamAngle() const
                    {
                        return equivalentBeamAngle;
                    }
                    
                    float TotalTransmitDuration() const
                    {
                        return pulseDuration;
                    }
                    
                    sonar::BeamGeometry BeamGeometry() const
                    {
                        return beamGeometry;
                    }
                    

            };

            //typedef sonar::Channel<ChannelData> Channel;
            //typedef sonar::Ping<std::vector<Channel>,nav::Fix> Ping;

        }
    }
}
#endif

