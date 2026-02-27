#ifndef GZ4D_SONAR_SIMRAD_FILE_CHANNELDATA_H
#define GZ4D_SONAR_SIMRAD_FILE_CHANNELDATA_H

#include <gz4d/sonar/Simrad/file/Datagram.h>
#include <gz4d/sonar/Simrad/file/FileFormat.h>
#include <gz4d/sonar/Simrad/Sample.h>
#include <gz4d/sonar/Channel.h>
#include <gz4d/sonar/Ping.h>
#include <gz4d/sonar/PingData.h>
#include <gz4d/time.h>
#include <gz4d/nav/Fix.h>

namespace gz4d
{
    namespace Simrad
    {
        namespace file
        {
            class ChannelData
            {
                DatagramWrapper<ConfigurationDatagram> config;
                DatagramWrapper<SampleDatagram> samples;

                float totalTransmitDuration;

                uint16_t cid;
                float G0; ///< Peak gain
                float Sa;

                public:
                    typedef std::shared_ptr<ChannelData> Ptr;
                    typedef SampleAdaptors<ChannelData> functor_types;
                    typedef int16_t* iterator;

                    ChannelData(){}
                    ChannelData(DatagramWrapper<ConfigurationDatagram> config, DatagramWrapper<SampleDatagram> samples, float ttd)
                    :config(config),samples(samples),totalTransmitDuration(ttd)
                    {
                        cid = samples->Channel -1;
                        
                        G0 = config->Beam[cid].Gain;
                        Sa = 0;
                        
                        for(int i = 0; i < 5; ++i)
                        {
                            if (config->Beam[cid].PulseLengthTable[i] == samples->PulseLength)
                            {
                                G0 = config->Beam[cid].GainTable[i];
                                Sa = config->Beam[cid].SaCorrectionTable[i];
                                break;
                            }
                        }
                        
                    }
                    

                    iterator begin()
                    {
                        return samples->Power;
                    }
                    
                    iterator end()
                    {
                        return samples->Power + samples->Count;
                    }
                    

                    uint16_t ID() const
                    {
                        return cid;
                    }
                    
                    
                    gz4d::Time Timestamp() const
                    {
                        return gz4d::time::FromNT(samples->DgHeader.DateTime);
                    }
                    
                    float TransmitPower() const
                    {
                        return samples->TransmitPower;
                    }
                    
                    float PeakGain() const {return G0;}
                    
                    float Frequency() const
                    {
                        return samples->Frequency;
                    }
                    
                    float SoundSpeed() const
                    {
                        return samples->SoundVelocity;
                    }
                    
                    float PulseDuration() const
                    {
                        return samples->PulseLength;
                    }
                    
                    float AbsorptionCoefficient() const
                    {
                        return samples->AbsorptionCoefficient;
                    }
                    
                    float SaCorrection() const {return Sa;}
                    float SampleInterval() const
                    {
                        return samples->SampleInterval;
                    }
                    
                    float SampleOffset() const
                    {
                        return samples->Offset*samples->SampleInterval;
                    }
                    
                    float EquivalentBeamAngle() const
                    {
                        //return pow(10.0,config->Beam[ID()].EquivalentBeamAngle/10.0);
                        return config->Beam[ID()].EquivalentBeamAngle;
                    }
                    
                    float TotalTransmitDuration() const
                    {
                        if(totalTransmitDuration > 0.0)
                            return totalTransmitDuration;
                        return samples->PulseLength;
                    }
                    
                    
                    sonar::BeamGeometry BeamGeometry() const
                    {
                        sonar::BeamGeometry ret;
                        ret.direction[0] = config->Beam[ID()].BeamSteeringAngleAthwartship;
                        ret.direction[1] = config->Beam[ID()].BeamSteeringAngleAlongship;
                        ret.width[0] = config->Beam[ID()].BeamWidthAthwartship;
                        ret.width[1] = config->Beam[ID()].BeamWidthAlongship;
                        return ret;
                    }
                    

            };

            typedef sonar::Channel<ChannelData> Channel;
            typedef sonar::Ping<sonar::PingData<Channel>,nav::Fix> Ping;

        }
    }
}
#endif

