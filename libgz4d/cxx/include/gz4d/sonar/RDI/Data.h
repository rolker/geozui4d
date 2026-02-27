#ifndef GZ4D_SONAR_RDI_DATA_H
#define GZ4D_SONAR_RDI_DATA_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/RDI/Ensemble.h>
#include <gz4d/sonar/RDI/FileFormat.h>
#include <gz4d/time.h>
#include <gz4d/sonar/BeamGeometry.h>
#include <gz4d/util/Swap.h>

namespace gz4d
{
    namespace RDI
    {
        struct BottomTrack
        {
            struct Beam
            {
                float velocity;
                float range;
                float percentGood;
                Beam()
                :velocity(Nan<float>()),range(Nan<float>()),percentGood(0.0)
                {}
            };
            Beam beams[4];
            operator bool(){for(int i = 0; i < 4; ++i) if(beams[i].percentGood == 0.0) return false; return true;}
        };

        typedef Vector<float,4> VelocityValues;

        class Data
        {
            protected:

                EnsembleWrapper ensemble;
                int year;
                gz4d::Time timestamp;
    
                int BDC(uint8_t value) const
                {
                    int ret = ((value&0xf0)>>4)*10+(value&0x0f);
                    return ret;
                }
                
                VelocityValues DecodeVelocities(uint8_t *data) const
                {
                    int16_t beam[4];
                    beam[0] = data[0] << 4 | (data[1] & 0xf0) >> 4;
                    beam[1] = (data[1] & 0x0f) << 8 | data[2] >> 4;
                    beam[2] = data[3] << 4 | (data[4] & 0xf0) >> 4;
                    beam[3] = (data[4] & 0x0f) << 8 | data[5] >> 4;
                    
                    for(int i = 0; i < 4; ++i)
                        if(beam[i] & 0x0800)
                            beam[i] |= 0xf000;
                        else
                            beam[i] &= 0x0fff;
                        
                        float scale = 0.005;
                    if(getConfig().coordinateFrame == Config::Beam_Coordinates)
                        scale /= 2.0;
                    if(getConfig().velocityRangeHigh && Frequency() != 76800.0)
                        scale /= 2.0;
                    
                    VelocityValues ret;
                    
                    bool status = hasStatus();
                    
                    for(int i = 0; i < 4; ++i)
                        if(!status && ((beam[i]&0x0fff) == 0x0800))
                            ret[i] = Nan<float>();
                        else
                            ret[i] = beam[i]*scale;
                        return ret;
                }
                
                VelocityValues DecodeVelocities(int16_t * const v) const
                {
                    VelocityValues ret;
                    for(int i = 0; i < 4; ++i)
                        if(v[i] == -32768)
                            ret[i] = Nan<float>();
                        else
                            ret[i] = v[i]*0.001;
                        return ret;
                }
                

                Data(){}
                Data(EnsembleWrapper ensemble, int year, gz4d::Time t)
                :ensemble(ensemble),year(year),timestamp(t)
                {
                    if(ensemble.old())
                        config = Config(ensemble.old()->leader.adcp_config);
                    else
                        config =  Config(ensemble.GetData<FixedLeader>()->SystemConfiguration,ensemble.GetData<FixedLeader>()->CoordinateTransformation);
                }
                
                Data(Data const &d)
                :ensemble(d.ensemble),year(d.year),timestamp(d.timestamp),config(d.config)
                {
                }
                
            public:
                gz4d::Time Timestamp() const
                {
                    if(timestamp.valid())
                    {
                        return timestamp;
                    }
                    if(ensemble.old())
                        return gz4d::Time(year, BDC(ensemble.old()->leader.rtc_month), BDC(ensemble.old()->leader.rtc_day), BDC(ensemble.old()->leader.rtc_hour), BDC(ensemble.old()->leader.rtc_minute), BDC(ensemble.old()->leader.rtc_second));
                    VariableLeader *vl = ensemble.GetData<VariableLeader>();
                    return gz4d::Time(2000+vl->RTCYear,vl->RTCMonth,vl->RTCDay,vl->RTCHour,vl->RTCMinute,vl->RTCSecond+vl->RTCHundreths/100.0);
                }
                
                float Frequency() const
                {
                    return getConfig().frequency;
                }
                
                float SoundSpeed() const
                {
                    if(ensemble.old())
                        return 1500.0;
                    return ensemble.GetData<VariableLeader>()->SpeedOfSound;
                }
                
                float PulseDuration() const
                {
                    if(ensemble.old())
                        return ensemble.old()->leader.pulse_length/(config.cangle*1475.1);
                    return ensemble.GetData<FixedLeader>()->XmitPulseLength*0.01/(config.cangle*ensemble.GetData<VariableLeader>()->SpeedOfSound);
                }
                
                float PulseLength() const
                {
                    if(ensemble.old())
                        return ensemble.old()->leader.pulse_length;
                    return ensemble.GetData<FixedLeader>()->XmitPulseLength*0.01;
                }
                
                float AbsorptionCoefficient() const
                {
                    return 0.044; // from rdi docs for 150 khz
                }
                
                float SampleInterval() const
                {
                    if(ensemble.old())
                        return 2.0*(1<<ensemble.old()->leader.bin_length)/(config.cangle*1475.1);
                    return 2.0*ensemble.GetData<FixedLeader>()->DepthCellLength*0.01/(config.cangle*ensemble.GetData<VariableLeader>()->SpeedOfSound);
                }
                
                float SampleOffset() const
                {
                    if(ensemble.old())
                        return (SampleInterval()/2.0)+2.0*(ensemble.old()->leader.pulse_length+ensemble.old()->leader.blank_after_transmit+ensemble.old()->leader.delay_after_blank)/(config.cangle*1475.1);
                    return 2.0*ensemble.GetData<FixedLeader>()->Bin1Distance*0.01/(config.cangle*ensemble.GetData<VariableLeader>()->SpeedOfSound);
                    
                }
                
                float EquivalentBeamAngle() const
                {
                    return 0.006303475; 
                }
                
                float TotalTransmitDuration() const
                {
                    if(ensemble.old())
                        return ensemble.old()->leader.pulse_length/(config.cangle*1475.1);
                    return ensemble.GetData<FixedLeader>()->XmitPulseLength*0.01/(config.cangle*ensemble.GetData<VariableLeader>()->SpeedOfSound);
                }
                
                float Temperature() const
                {
                    if(ensemble.old())
                        return 45.0-(50.0*Swap(ensemble.old()->leader.adcp_temperature)/4096.0);
                    return 0.0;
                }
                
                float TransmitPower() const
                {
                    return 53.3988; // k1 for 150 VM
                }
                
                sonar::BeamGeometry BeamGeometry(uint8_t channel) const
                {
                    sonar::BeamGeometry ret;
                    ret.width[0] = 4.0;
                    ret.width[1] = 4.0;
                    
                    float angle = getConfig().angle;
                    if (getConfig().transducerConcave)
                        angle = -angle;
                    
                    switch(channel)
                    {
                        case 0:
                            ret.direction[0] = -angle;
                            ret.direction[1] = 0.0;
                            break;
                        case 1:
                            ret.direction[0] = angle;
                            ret.direction[1] = 0.0;
                            break;
                        case 2:
                            ret.direction[0] = 0.0;
                            ret.direction[1] = angle;
                            break;
                        case 3:
                            ret.direction[0] = 0.0;
                            ret.direction[1] = -angle;
                            break;
                        default:
                            break;
                    }
                    return ret;
                }
                

                struct Config
                {
                    enum CoordinateFrame { Beam_Coordinates=0, Instrument_Coordinates=1, Ship_Coordinates=2, Earth_Coordinates=3};
                    bool velocityRangeHigh;
                    CoordinateFrame coordinateFrame;
                    bool attitudeCompensated;
                    bool transducerDownLooking;
                    bool transducerConcave;
                    float frequency;
                    bool valid;
                    float angle;
                    float cangle;
                    float sangle;

                    Config(uint8_t config_byte)
                    {
                        velocityRangeHigh = config_byte & 0x01;
                        if(config_byte & 0x02)
                        {
                            coordinateFrame = Earth_Coordinates;
                            attitudeCompensated = true;
                        }
                        else
                        {
                            coordinateFrame = Beam_Coordinates;
                            attitudeCompensated = false;
                        }
                        
                        transducerDownLooking = config_byte & 0x04;
                        transducerConcave = config_byte & 0x08;
                        angle = 30.0;
                        cangle = cos(Radians(angle));
                        sangle = sin(Radians(angle));
                        
                        switch(config_byte &0x70)
                        {
                            case 0x00:
                                frequency = 76800.0;
                                break;
                            case 0x10:
                                frequency = 153600.0;
                                break;
                            case 0x20:
                                frequency = 307200.0;
                                break;
                            case 0x30:
                                frequency = 614400.0;
                                break;
                            case 0x40:
                                frequency = 1228800.0;
                                break;
                            case 0x50:
                                frequency = 115000.0;
                                break;
                            default:
                                frequency=0.0;
                        }
                        valid = config_byte & 0x80;
                    }
                    
                    Config(uint16_t config_word, uint8_t ex_byte)
                    {
                        velocityRangeHigh = false;
                        switch((ex_byte | 0x18)>>3)
                        {
                            case Beam_Coordinates:
                                coordinateFrame = Beam_Coordinates;
                                break;
                            case Instrument_Coordinates:
                                coordinateFrame = Instrument_Coordinates;
                                break;
                            case Ship_Coordinates:
                                coordinateFrame = Ship_Coordinates;
                                break;
                            case Earth_Coordinates:
                                coordinateFrame = Earth_Coordinates;
                                break;
                            default:
                                coordinateFrame = Beam_Coordinates;
                        }
                        
                        attitudeCompensated = ex_byte & 0x04;
                        transducerDownLooking = !(config_word & 0x80);
                        transducerConcave = !(config_word & 0x08);
                        switch(config_word&0x0300)
                        {
                            case 0x0000:
                                angle = 15.0;
                                break;
                            case 0x0100:
                                angle = 20.0;
                                break;
                            case 0x0200:
                                angle = 30.0;
                                break;
                            default:
                                angle = Nan<float>();
                        }
                        cangle = cos(Radians(angle));
                        sangle = sin(Radians(angle));
                        
                        switch(config_word &0x07)
                        {
                            case 0x00:
                                frequency = 76800.0;
                                break;
                            case 0x01:
                                frequency = 153600.0;
                                break;
                            case 0x02:
                                frequency = 307200.0;
                                break;
                            case 0x03:
                                frequency = 614400.0;
                                break;
                            case 0x04:
                                frequency = 1228800.0;
                                break;
                            case 0x05:
                                frequency = 2457600.0;
                                break;
                            case 0x06:
                                frequency = 38400.0;
                                break;
                            default:
                                frequency=0.0;
                        }
                        valid = true;
                    }
                    
                    Config()
                    :valid(false)
                    {
                    }
                    
                };

                Config const &getConfig() const
                {
                    return config;
                }
                


                BottomTrack GetBottomTrack() const
                {
                    if(ensemble.old())
                    {
                        VelocityValues v = DecodeVelocities(ensemble.old()->leader.bottom_tracking_velocity);
                        BottomTrack ret;
                        for(int i = 0; i < 4; ++i)
                        {
                            ret.beams[i].velocity = v[i];
                            ret.beams[i].range = Swap(ensemble.old()->leader.beam_range[i]);
                        }
                        
                        ret.beams[0].percentGood = (ensemble.old()->leader.bottom_tracking_pg[0] >> 4)/.15;
                        ret.beams[1].percentGood = (ensemble.old()->leader.bottom_tracking_pg[0] & 0x0f)/.15;
                        ret.beams[2].percentGood = (ensemble.old()->leader.bottom_tracking_pg[1] >> 4)/.15;
                        ret.beams[3].percentGood = (ensemble.old()->leader.bottom_tracking_pg[1] & 0x0f)/.15;
                        
                        return ret;
                    }
                    BottomTrack ret;
                    if(ensemble.GetData<BottomTrackData>())
                    {
                        VelocityValues v = DecodeVelocities(ensemble.GetData<BottomTrackData>()->Velocity);
                        for(int i = 0; i < 4; ++i)
                        {
                            ret.beams[i].velocity = v[i];
                            ret.beams[i].range = (ensemble.GetData<BottomTrackData>()->Range[i]+ensemble.GetData<BottomTrackData>()->RangeMSB[i]*0x10000)/100.0;
                            ret.beams[i].percentGood = ensemble.GetData<BottomTrackData>()->Correlation[i];
                        }
                    }
                    return ret;
                }
                

                //uint8_t VelocityByte(std::size_t i, uint8_t j);
                int PingsPerEnsemble() const
                {
                    if(ensemble.old())
                        return Swap(ensemble.old()->leader.ping_per_ensemble);
                    return ensemble.GetData<FixedLeader>()->NumberOfPings;
                }
                

                std::string Info() const
                {
                    std::stringstream ret;
                    if(ensemble.old())
                        ret << "sizes (velocity, spectral, echo, percentGood, status): " << Swap(ensemble.old()->header.VelocitySize) << ", "
                        << Swap(ensemble.old()->header.SpectralSize) << ", "
                        << Swap(ensemble.old()->header.EchoSize) << ", "
                        << Swap(ensemble.old()->header.PercentGoodSize) << ", "
                        << Swap(ensemble.old()->header.StatusSize) << std::endl;
                    else
                        ret << "sizes (velocity, correleation mag, echo, percentGood): " << ensemble.GetDataSize<VelocityData>() << ", "
                        << ensemble.GetDataSize<CorrelationMagnitudeData>() << ", "
                        << ensemble.GetDataSize<EchoIntensityData>() << ", "
                        << ensemble.GetDataSize<PercentGoodData>() << std::endl;
                    return ret.str();
                }
                

                bool hasPercentGood() const
                {
                    if(ensemble.old())
                        return Swap(ensemble.old()->header.PercentGoodSize);
                    return ensemble.GetData<PercentGoodData>();
                }
                
                bool hasStatus() const
                {
                    if(ensemble.old())
                        return Swap(ensemble.old()->header.StatusSize);
                    return false;
                }
                
            protected:
                Config config;
        };
    }
}

#endif
